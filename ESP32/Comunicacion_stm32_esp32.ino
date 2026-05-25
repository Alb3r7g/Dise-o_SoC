#include <PubSubClient.h>
#include <WiFi.h>


// Red
const char *ssid = "MELON 9407";
const char *password = "N40k8>62";

// IP de la raspberry pi
const char *mqtt_server = "192.168.137.160";
const int mqtt_port = 1883;

// Tópico MQTT
const char *mqtt_topic = "tractor/telemetria";

WiFiClient espClient;
PubSubClient client(espClient);

// Pines seriales para comunicación con STM32
#define RXD2 16
#define TXD2 17

String tramaSerial = "";

// Control de tiempo para mensajes de estado
unsigned long ultimoMensajeEstado = 0;
unsigned long ultimaTramaRecibida = 0;
bool recibiendoDatos = false;

void setup_wifi() {
  delay(10);
  Serial.println("\nConectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nconectado ip: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  Serial.println(" ESP32 conectada.");
  Serial.println(" Esperando datos de STM32");
}

void loop() {
  unsigned long ahora = millis();

  // cada 2 segundos si no llegan datos
  if (ahora - ultimoMensajeEstado >= 2000) {
    ultimoMensajeEstado = ahora;
    if (!recibiendoDatos) {
      Serial.println("Esperando datos de la STM32");
    } else {
      // Comprobamos si dejaron de llegar datos
      if (ahora - ultimaTramaRecibida > 500) {
        Serial.println("STM32 dejó de enviar datos");
        recibiendoDatos = false;
      }
    }
  }

  // Leectura los datos
  while (Serial2.available() > 0) {
    char c = Serial2.read();

    if (c == '\n') {
      if (tramaSerial.length() > 0) {
        // Marcamos que estamos recibiendo datos
        recibiendoDatos = true;
        ultimaTramaRecibida = ahora;

        Serial.print("RPM,V,G -> ");
        Serial.println(tramaSerial);

        // Solo publicamos si hay conexión MQTT
        if (client.connected()) {
          client.publish(mqtt_topic, tramaSerial.c_str());
          Serial.println("        -> Dato enviado");
        } else {
          Serial.println("        -> Dato no enviado, MQTT desactivado.");
        }

        tramaSerial = "";
      }
    } else if (c != '\r') {
      tramaSerial += c;
    }
  }

  // MQTT
  if (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Conectados al broker (Raspberry Pi)");
      // Este mensaje solo fue una de nuestras prubas para comprobar
      // funcionamiento
      client.publish(mqtt_topic, "0,0,0");
    }
  }
  client.loop();
}
