#include <PubSubClient.h>
#include "main.h"
#include "../led/rgb.h"
#include "wifi/wifi.h"

WiFiClient espClient;
PubSubClient client(espClient);

void mqtt_init(const char* mqtt_server, const int mqtt_port) {
    // Inicia servidor MQTT
    client.setServer(mqtt_server, mqtt_port);
}

void mqtt_reconnect() {
    // Serial.print("Reconnecting to MQTT broker...");
    // Inicia bucle hasta que se conecte con el broker
    while (!client.connected()) {
        // intenta conectar al servidor MQTT
        if (client.connect("nodemcu-client")) {
            // Serial.println("Connected to MQTT broker");
        } else {
            Serial.print("Failed to connect to MQTT broker, retrying in 5 seconds...");
            delay(5000);
        }
    }
}

void mqtt_is_connected() {
    turnOffLED(pinRojo, pinVerde, pinAzul);
    commandLED(1023, 0, 0, pinRojo, pinVerde, pinAzul);

    // Comprueba si se encuentra conectado con el broker
    mqtt_reconnect();

    turnOffLED(pinRojo, pinVerde, pinAzul);
    commandLED(0, 20, 0, pinRojo, pinVerde, pinAzul);

    // Inicia servidor de forma indefinida
    client.loop();
}
