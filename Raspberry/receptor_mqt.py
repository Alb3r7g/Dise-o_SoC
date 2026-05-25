import paho.mqtt.client as mqtt
import csv
import os
from datetime import datetime
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

#Nuestras llaves y configuracion
INFLUX_TOKEN = "rfxAIyr4V8zIpavmvmWmZb7iTb0V57Idw6z-yck9jPy59pjN3i3p3i8QhukYLTtiIo5JYSdQwYrdMSXUoQeY-g=="
INFLUX_ORG = "FacultadIng" 
INFLUX_URL = "http://localhost:8086"
INFLUX_BUCKET = "Reto_SoC"
MQTT_BROKER = "localhost" 
MQTT_TOPIC = "tractor/telemetria"

#Archivo CSV
CSV_DIR = "data"
CSV_FILE = f"{CSV_DIR}/datos_tractor.csv"

# Creamos directorio si no existe (para la primera vez)
if not os.path.exists(CSV_DIR):
    os.makedirs(CSV_DIR)

# Encabezados segun los requerimientos del reto
if not os.path.isfile(CSV_FILE):
    with open(CSV_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Velocidad del motor", "Velocidad del vehiculo", "Marcha"])

# InfluxDB
influx_client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

#MQTT
def on_connect(client, userdata, flags, rc):
    print(f"Conectado al Broker MQTT. Codigo: {rc}")
    client.subscribe(MQTT_TOPIC)
    print(f"Suscrito al topico: {MQTT_TOPIC}")

def on_message(client, userdata, msg):
    try:
        # RPM,Velocidad,Marcha
        payload = msg.payload.decode('utf-8').strip()
        valores = payload.split(',')
        
        if len(valores) == 3:
            vel_motor = int(valores[0])
            vel_vehiculo = float(valores[1])
            marcha = int(valores[2])
            t_now = datetime.now()
            timestamp_str = t_now.strftime("%Y-%m-%d %H:%M:%S")

            # Guardamos todo en el csv
            with open(CSV_FILE, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([vel_motor, vel_vehiculo, marcha])

            # lo mandamos a influxdb
            punto = Point("telemetria") \
                .field("velocidad_motor", vel_motor) \
                .field("velocidad_vehiculo", vel_vehiculo) \
                .field("marcha", marcha) \
                .time(datetime.utcnow(), WritePrecision.NS)
            
            write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=punto)
            
            print(f"Motor: {vel_motor} | Vehiculo: {vel_vehiculo} | Marcha: {marcha}")

    except Exception as e:
        print(f"Error en procesamiento: {e}")

#Inicio del servicio
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_BROKER, 1883, 60)
mqtt_client.loop_forever()