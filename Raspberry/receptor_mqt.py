import paho.mqtt.client as mqtt
import csv
import os
from datetime import datetime
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS

# Aqui definimos las credenciales de InfluxDB y la configuracion de MQTT
INFLUX_TOKEN = "rfxAIyr4V8zIpavmvmWmZb7iTb0V57Idw6z-yck9jPy59pjN3i3p3i8QhukYLTtiIo5JYSdQwYrdMSXUoQeY-g=="
INFLUX_ORG = "FacultadIng" 
INFLUX_URL = "http://localhost:8086"
INFLUX_BUCKET = "telemetria_tractor"
MQTT_BROKER = "localhost" 
MQTT_TOPIC = "tractor/telemetria"

DEVICE_ID = "STM32_01"
OPERATOR_ID = "operador_01"

# Aqui especificamos la ruta del archivo CSV para el registro local
CSV_DIR = "data"
CSV_FILE = f"{CSV_DIR}/datos_tractor.csv"

# Aqui creamos el directorio de almacenamiento si no existe
if not os.path.exists(CSV_DIR):
    os.makedirs(CSV_DIR)

# Aqui escribimos la cabecera del archivo CSV si es un archivo nuevo
if not os.path.isfile(CSV_FILE):
    with open(CSV_FILE, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(["Velocidad del motor", "Velocidad del vehiculo", "Marcha"])

# Aqui inicializamos el cliente y la API de escritura para InfluxDB
influx_client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

# Aqui definimos las funciones de retorno para la conexion y recepcion de mensajes MQTT
def on_connect(client, userdata, flags, rc):
    print(f"Conectado al Broker MQTT. Codigo: {rc}")
    client.subscribe(MQTT_TOPIC)
    print(f"Suscrito al topico: {MQTT_TOPIC}")

def on_message(client, userdata, msg):
    try:
        # Decodificamos y separamos los valores recibidos de telemetria (RPM, Velocidad, Marcha)
        payload = msg.payload.decode('utf-8').strip()
        valores = payload.split(',')
        
        if len(valores) == 3:
            vel_motor = int(valores[0])
            vel_vehiculo = float(valores[1])
            marcha = int(valores[2])
            t_now = datetime.now()
            timestamp_str = t_now.strftime("%Y-%m-%d %H:%M:%S")

            # Aqui agregamos los nuevos datos al final del archivo CSV
            with open(CSV_FILE, mode='a', newline='') as file:
                writer = csv.writer(file)
                writer.writerow([vel_motor, vel_vehiculo, marcha])

            # Aqui enviamos las metricas de telemetria a la base de datos InfluxDB
            punto = Point("telemetria_remota_tractor") \
                .tag("id_dispositivo", DEVICE_ID) \
                .tag("operador_id", OPERATOR_ID) \
                .field("rpm", vel_motor) \
                .field("velocidad", vel_vehiculo) \
                .field("marcha", marcha) \
                .time(datetime.utcnow(), WritePrecision.NS)
            
            write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=punto)
            
            print(f"Motor: {vel_motor} | Vehiculo: {vel_vehiculo} | Marcha: {marcha}")

    except Exception as e:
        print(f"Error en procesamiento: {e}")

# Aqui inicializamos el cliente MQTT y arrancamos el bucle de escucha continuo
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_BROKER, 1883, 60)
mqtt_client.loop_forever()
