#include <PubSubClient.h>
#include "main.h"
#include "../led/rgb.h"
#include "wifi/wifi.h"

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_init(const char* mqtt_server, const int mqtt_port) {
    client.setServer(mqtt_server, mqtt_port);
}

void mqtt_reconnect() { // funcion que se encarga de reconectarse de nuevo...
  // loop hasta que nos podamos conectar al servidor MQTT
  while (!client.connected()) { // si no se encuentra conectado ...PubSubClient client(espClient);
    Serial.print("Intentando reconectar...");
    // intenta conectar al servidor MQTT
    if (client.connect("nodemcu-client")) {
      // nos hemos conectado al servidor MQTT
    } else {
      // error al conectar, esperamos y lo intentamos de nuevo
      delay(5000);
    }
  }
}

void mqtt_is_connected(){
    turnOffLED(pinRojo, pinVerde, pinAzul);
    commandLED(1023, 0, 0, pinRojo, pinVerde, pinAzul);

    mqtt_reconnect();

    turnOffLED(pinRojo, pinVerde, pinAzul);
    commandLED(0, 20, 0, pinRojo, pinVerde, pinAzul);
    client.loop();
}