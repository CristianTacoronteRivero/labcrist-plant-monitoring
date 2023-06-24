#ifndef MQTT_H
#define MQTT_H

/**
 * @brief Inicializa la conexión MQTT con el servidor especificado.
 *
 * @param mqtt_server Dirección IP del servidor MQTT.
 * @param mqtt_port Puerto utilizado por el servidor MQTT.
 */
void mqtt_init(const char* mqtt_server, const int mqtt_port);

/**
 * @brief Verifica si el cliente MQTT está conectado. Si no está conectado, se intenta reconectar.
 *
 * Esta función verifica si el cliente MQTT está conectado. Si no lo está, intenta reconectar
 * al servidor MQTT utilizando la función mqtt_reconnect(). También se encarga de mantener
 * la conexión activa y procesar los mensajes entrantes utilizando la función client.loop().
 *
 * @see mqtt_reconnect()
 */
void mqtt_is_connected();

/**
 * @brief Función que se encarga de reconectar al servidor MQTT si no se encuentra conectado.
 *
 * Esta función se encarga de intentar reconectar al servidor MQTT si el cliente no se encuentra
 * conectado. Si se logra conectar, se informa por el monitor serial.
 */
void mqtt_reconnect();

/**
 * @brief Cliente MQTT utilizado para la conexión con el servidor.
 *
 * Este objeto cliente MQTT puede ser utilizado en otras partes del código para publicar o
 * recibir mensajes del servidor MQTT.
 *
 * @note Este objeto es declarado como extern para que pueda ser utilizado en otras partes del código.
 */
extern PubSubClient client;

#endif // MQTT_H
