#include <PubSubClient.h>
#include <WiFi.h>

// red
const char *ssid = "MELON 9407";
const char *password = "N40k8>62";

// ip de la raspberry pi
const char *mqtt_server = "192.168.137.160";
const int mqtt_port = 1883;

// tópicos MQTT
const char *mqtt_topic = "tractor/telemetria";
const char *mqtt_topic_control =
    "tractor/control"; // Tópico para recibir comandos del dashboard

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
  Serial.println("\nconectando a wifi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nconectado ip: " + WiFi.localIP().toString());
}

// callback para recibir mensajes MQTT
void callback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, mqtt_topic_control) == 0) {
    // aqui construimos la cadena de mensaje recibida
    String rawMsg = "";
    for (int i = 0; i < length; i++) {
      rawMsg += (char)payload[i];
    }

    // aqui construimos la trama para la STM32 con el prefijo "C:"
    String msg = "C:" + rawMsg + "\n";

    // enviamos por UART a la STM32
    Serial2.print(msg);

    // imprimimos en la terminal de esp32 lo que nos manda la raspi, para
    // verificar la conexión
    Serial.println();
    Serial.println("-----------------------");
    Serial.print("Remoto (1|0), Aceleracion (%): ");
    Serial.println(rawMsg);
    Serial.println();
    Serial.println();
    Serial.println();

    // depuración
    int commaIndex = rawMsg.indexOf(',');
    if (commaIndex != -1) {
      String strEstado = rawMsg.substring(0, commaIndex);
      String strValor = rawMsg.substring(commaIndex + 1);

      int estadoVal = strEstado.toInt();
      int aceleracionVal = strValor.toInt();

      Serial.print("Modo control remoto: ");
      if (estadoVal == 1) {
        Serial.println("Habilitado - potenciometro desactivado");
      } else {
        Serial.println("Deshabilitado - potenciometro activado");
      }

      Serial.print("Aceleracion:   ");
      Serial.print(aceleracionVal);
      Serial.println("%");
    } else {
      Serial.println("Formato de payload no reconocido (falta coma).");
    }

    Serial.print("Mensaje UART: \"");
    String visualMsg = msg;
    visualMsg.replace("\n", "\\n");
    Serial.print(visualMsg);
    Serial.println("\"");
    Serial.println("-----------------------");
    Serial.println();
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Registramos función callback

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
          Serial.println(
              "        -> Dato no enviado, el MQTT esta desactivado.");
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

      // Suscripción al tópico de control apenas nos conectamos
      client.subscribe(mqtt_topic_control);
      Serial.println("Suscrito a tractor/control");

      // Este mensaje solo fue una de nuestras prubas para comprobar
      // funcionamiento
      client.publish(mqtt_topic, "0,0,0");
    }
  }
  client.loop();
}
