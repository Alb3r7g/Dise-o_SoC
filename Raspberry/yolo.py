# -*- coding: utf-8 -*-
import cv2
import urllib.request
import numpy as np
from flask import Flask, Response
from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime
import time
import threading

app = Flask(__name__)

# Aquí definimos las variables para compartir fotogramas entre el hilo de procesamiento y Flask
frame_actual = None
lock_frame = threading.Lock()

# Aquí especificamos la dirección URL de captura de la cámara
url = 'http://192.168.137.200/capture'

# Aquí configuramos las credenciales y el cliente para InfluxDB
INFLUX_TOKEN = "rfxAIyr4V8zIpavmvmWmZb7iTb0V57Idw6z-yck9jPy59pjN3i3p3i8QhukYLTtiIo5JYSdQwYrdMSXUoQeY-g=="
INFLUX_ORG = "FacultadIng" 
INFLUX_URL = "http://localhost:8086"
INFLUX_BUCKET = "Reto_SoC"

influx_client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

# Aquí inicializamos el modelo de detección YOLOv4-Tiny y leemos las clases
with open("coco.names", "r") as f:
    classes = [line.strip() for line in f.readlines()]

net = cv2.dnn.readNet("yolov4-tiny.weights", "yolov4-tiny.cfg")
model = cv2.dnn_DetectionModel(net)
model.setInputParams(size=(320, 320), scale=1/255, swapRB=True)

CONF_THRESHOLD = 0.65
NMS_THRESHOLD = 0.4

def procesamiento_yolo_background():
    global frame_actual
    ultimo_envio = 0
    ultimo_estado = -1
    
    print("Hilo de procesamiento YOLO iniciado.")
    while True:
        try:
            # Aquí obtenemos la imagen desde la cámara web
            img_resp = urllib.request.urlopen(url, timeout=1)
            img_bytes = img_resp.read()
            img_np = np.frombuffer(img_bytes, dtype=np.uint8)
            frame = cv2.imdecode(img_np, cv2.IMREAD_COLOR)
            
            if frame is None:
                continue

            # Aquí procesamos el fotograma usando el modelo de red neuronal
            classes_detected, confidences, boxes = model.detect(frame, CONF_THRESHOLD, NMS_THRESHOLD)

            estado_deteccion = 0

            # Aquí dibujamos los rectángulos sobre los objetos detectados
            for (class_id, score, box) in zip(classes_detected, confidences, boxes):
                class_name = classes[int(class_id)]
                
                # Aquí identificamos personas o sillas como obstáculos de seguridad
                if class_name in ["person", "chair"]:
                    estado_deteccion = 1 
                
                x, y, w, h = box
                # Aquí asignamos el color de marcado según el tipo de objeto detectado
                color = (0, 0, 255) if class_name in ["person", "chair"] else (0, 255, 0)
                
                cv2.rectangle(frame, (x, y), (x + w, y + h), color, 2)
                cv2.putText(frame, class_name, (x, y - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)

            # Aquí registramos la detección en InfluxDB
            ahora = time.time()
            # Aquí actualizamos la base de datos de manera controlada para evitar saturación
            if estado_deteccion != ultimo_estado or (ahora - ultimo_envio > 1.0):
                punto = Point("seguridad_entorno") \
                    .field("estado_deteccion", estado_deteccion) \
                    .time(datetime.utcnow(), WritePrecision.NS)
                
                write_api.write(bucket=INFLUX_BUCKET, org=INFLUX_ORG, record=punto)
                
                ultimo_estado = estado_deteccion
                ultimo_envio = ahora

            # Aquí codificamos el fotograma procesado en formato JPEG
            ret, buffer = cv2.imencode('.jpg', frame)
            if ret:
                frame_bytes = buffer.tobytes()
                # Aquí actualizamos la variable global usando exclusión mutua por hilos
                with lock_frame:
                    frame_actual = frame_bytes

        except Exception as e:
            print(f"Error procesando frame: {e}")
            time.sleep(1)

def generate_frames():
    # Aquí leemos el fotograma más reciente de forma segura
    while True:
        with lock_frame:
            if frame_actual is not None:
                frame_yield = frame_actual
            else:
                frame_yield = None
                
        if frame_yield is not None:
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame_yield + b'\r\n')
                   
        # Aquí controlamos la frecuencia de fotogramas para optimizar el ancho de banda
        time.sleep(0.05)

# Aquí definimos la ruta de Flask para transmitir el video continuo
@app.route('/video_feed')
def video_feed():
    return Response(generate_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')

if __name__ == '__main__':
    # Aquí arrancamos el hilo secundario para la detección de objetos y levantamos el servidor Flask
    hilo_yolo = threading.Thread(target=procesamiento_yolo_background, daemon=True)
    hilo_yolo.start()

    print("Servidor YOLO Flask iniciado en puerto 5000...")
    print("El video esta disponible en http://localhost:5000/video_feed")
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)
