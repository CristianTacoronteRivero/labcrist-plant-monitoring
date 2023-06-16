#include <ESP8266WiFi.h>
#include "main.h"

void setup_wifi(const char* ssid, const char* password, const char* ip_str, const char* gateway_str, const char* subnet_str) {
    Serial.println("Connecting to WiFi network...");

    // Convirte de str a tuple(int) los parametros WiFi del NodeMCU
    IPAddress ip;
    ip.fromString(ip_str);

    IPAddress gateway;
    gateway.fromString(gateway_str);

    IPAddress subnet;
    subnet.fromString(subnet_str);

    // Inicia la configuracion WiFi
    WiFi.config(ip, gateway, subnet);

    // Conecta con la red WiFi
    WiFi.begin(ssid, password);

    // Espera hasta que se conecte a la red
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print("Connected to WiFi network ");
    Serial.print(ssid);
    Serial.print(" with IP address: ");
    Serial.println(WiFi.localIP());
}
