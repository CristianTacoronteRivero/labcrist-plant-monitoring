#include <ESP8266WiFi.h>

void setup_wifi(const char* ssid, const char* password, const char* ip_str, const char* gateway_str, const char* subnet_str) {
    // demora en ms para dar tiempo a que se inicien los procesos
    delay(10);

    // bloque que convierte de str a tuple(int) //
    // IP deseada para el NodeMCU
    IPAddress ip;
    ip.fromString(ip_str);
    // Gateway de la red
    IPAddress gateway;
    // Mask de la red
    gateway.fromString(gateway_str);
    IPAddress subnet;
    subnet.fromString(subnet_str);

    // configura los parametros del NodeMCU
    WiFi.config(ip, gateway, subnet);

    // conecta a la red WiFi
    WiFi.begin(ssid, password);

    // espera hasta que se conecte a la red
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print("Conectando a la red WiFi...");
        delay(500);
    }
}