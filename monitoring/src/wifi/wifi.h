#ifndef WIFI_H
#define WIFI_H

#include <ESP8266WiFi.h>

/**
 * Configura la conexión WiFi del NodeMCU con la dirección IP, gateway y máscara de subred especificadas.
 * Conecta el NodeMCU a la red WiFi con el SSID y contraseña proporcionados.
 *
 * @param ssid El SSID de la red WiFi a la que se desea conectar.
 * @param password La contraseña de la red WiFi a la que se desea conectar.
 * @param ip_str La dirección IP deseada para el NodeMCU en formato de cadena de caracteres.
 * @param gateway_str La dirección IP del router en formato de cadena de caracteres.
 * @param subnet_str La máscara de subred en formato de cadena de caracteres.
 */
void setup_wifi(const char* ssid, const char* password, const char* ip_str, const char* gateway_str, const char* subnet_str);

#endif // WIFI_H
