"""
Realiza la suscripcion a uno o mas topics y procesa el payload de una forma determinada para, posteriormente,
almacenarlo en InfluxDB
"""
import logging
import os
import sys
import json
from configparser import ConfigParser
from time import sleep

import paho.mqtt.client as mqtt
from influxdb import InfluxDBClient
from paho.mqtt.properties import MQTTException
from requests.exceptions import ConnectionError as RequestsConnectionError

# Agrega la ruta al PYTHONPATH
sys.path.append("/home/labcrist/labcrist-plant-monitoring/raspberry")
sys.path.append("/home/ctacoronte/Escritorio/GitHub/raspberry-micgridblue")
sys.path.append(os.environ.get("PATH_MGB"))

from func import microgridblue as fn


class SubMqtt:
    """
    Clase que ofrece las funciones necesarias para suscribirse a uno o varios
    topics
    """

    def __init__(self, ip_address: str, client_influx: InfluxDBClient) -> None:
        """
        Constructor de la clase.

        :param ip_address: Dirección IP del broker MQTT.
        :type ip_address: str
        :param client_influx: Cliente InfluxDB para almacenar los datos recibidos.
        :type client_influx: InfluxDBClient
        :return: None
        """
        self.broker_address = ip_address
        self.client_influx = client_influx
        self.client = mqtt.Client(
            clean_session=True, userdata=None, protocol=mqtt.MQTTv311
        )
        # Asociar funcion __on_connect a on_connect
        self.client.on_connect = self.__on_connect
        # Asociar funcion __on_message a on_message
        self.client.on_message = self.__on_message
        # Asociar funcion __on_disconnect a on_disconnect
        self.client.on_disconnect = self.__on_disconnect

        # Conectar instancia
        self.client.connect(host=self.broker_address)

    def __on_connect(self, client, userdata, flags, rc):
        """Registra el estado de la conexion"""
        if rc == 0:
            logging.info("Conexión establecida con el broker, esperando datos...")
        else:
            logging.error("Conexión no establecida con el broker")
            raise ValueError("Conexión no establecida con el broker")

    def subscribe_topics(self, topic: list[str], qos: int) -> None:
        """
        Suscribe el cliente MQTT a una lista de topics especificados.

        :param topic: Lista de topics a los que suscribirse.
        :type topic: list[str]
        :param qos: Nivel de calidad de servicio para la suscripción (QoS).
        :type qos: int
        :return: None
        """

        # Suscribir el servidor a los topics
        for t in topic:
            result, _ = self.client.subscribe(t.strip(), qos=qos)
            if result == 0:
                logging.info(f"Suscripción realizada con éxito: {t.strip()}")
            else:
                logging.error(f"Suscripción no realizada con éxito: {t.strip()}")

    def __on_message(self, client, userdata, msg):
        """
        Aplica una serie de operaciones al payload que recibe vía MQTT.

        :param client: Cliente MQTT que recibe el mensaje.
        :type client: paho.mqtt.client.Client
        :param userdata: Datos de usuario (opcional).
        :type userdata: Any
        :param msg: Mensaje MQTT recibido.
        :type msg: paho.mqtt.client.MQTTMessage
        :return: None
        """
        try:
            # Obtener el diccionario construido por el nodo
            # '{"temperatura":29.79999924,"humedad":48}' -> {"temperatura":29.79999924,"humedad":48}
            value = json.loads(msg.payload.decode())
            # Forzar a que sean un tipo de variable en concreto para evitar futuros errores
            for clave, valor in value.items():
                value[clave] = round(float(valor),1)

            measurement = msg.topic.split("/")[0]
            tag_sensor = msg.topic.split("/")[1]

            # Construir diccionario
            points = [
                {
                    "measurement": measurement,
                    "tags": {
                        "sensor": tag_sensor,
                    },
                    "fields": value
                }
            ]

            # Registrar datos en base datos local InfluxDB
            self.client_influx.write_points(points=points, time_precision="s")

            logging.info(points)
        except Exception as e:
            line_error, data_error = fn.traceback_logging()
            logging.error(
                f"Error en la función _on_message: {e}, linea {line_error} -> {data_error}"
            )

    def start_broker(self):
        """Inicia el broker para escuchar los topics subscritos
        y poder recibirlos. NOTA: este metodo es necesario solamente
        cuando se suscribe a un topic, no cuando se pretende enviar
        mensajes.
        """
        try:
            # Iniciar broker
            self.client.loop_forever()

        except KeyboardInterrupt:
            logging.error("Interrupción por el usuario")
            self.disconnect()

    def disconnect(self):
        """Para la conexion y desconecta la comunicacion con
        el broker
        """
        try:
            # Bloquear procedimiento
            self.client.loop_stop()
            # Desconectar servicio para ahorrar recursos
            self.client.disconnect()
            logging.info("Cliente desconectado de forma satisfactoria")
        except MQTTException as e:
            logging.error(f"Error al cerrar la conexión: {e}")
            raise

    def __on_disconnect(self, client, userdata, rc):
        """Desconecta el servicio. Este metodo esta pensado para ser
        lanzado de forma automatica al perderse la conexion y, posteriormente
        en el bucle while intenta conectarse de nuevo
        """
        logging.error("...Conexión finalizada")
        self.disconnect()


def connect_database() -> InfluxDBClient:
    """
    Establece una conexión con la base de datos InfluxDB y devuelve un objeto InfluxDBClient.

    :return: Cliente de InfluxDB para realizar operaciones en la base de datos.
    :rtype: InfluxDBClient
    """
    while True:
        try:
            client = InfluxDBClient(timeout=TIMEOUT)
            logging.info(
                "Establecimiento de conexion satisfactoria con la base de datos"
            )
            break
        except Exception as e:
            line_error, data_error = fn.traceback_logging()
            logging.error(
                f"No se pudo conectar con la base de datos InfluxDB: {e}, linea {line_error} -> {data_error}"
            )
            sleep(TIMEOUT)

    return client


if __name__ == "__main__":
    # Configurar y crear archivo logging
    fn.create_logging(os.path.basename(__file__))

    fn.logging.info("Script principal ejecutado".center(100, "*"))

    # Cargar la configuración desde el archivo .conf
    config = ConfigParser()
    config.read(fn.search_path_file("main.conf"))

    # Obtener los valores de la sección [MQTT]
    DATABASE = config.get("MQTT", "database")
    TIMEOUT = config.getint("MQTT", "timeout")
    TOPIC = config.get("MQTT", "topic").split(",")
    TABLE = config.get("MQTT", "table")
    BROKER = config.get("MQTT", "broker")
    QOS = config.getint("MQTT", "qos")

    logging.info(
        f"""Variables especificadas:
        \t\t\t\tdatabase: {DATABASE},
        \t\t\t\ttable: {TABLE},
        \t\t\t\ttopics: {TOPIC},
        \t\t\t\tbroker: {BROKER},
        \t\t\t\tqos: {QOS},
        \t\t\t\ttimeout: {TIMEOUT}"""
    )

    # Conectar con la base de datos local InfluxDB
    client_influx = connect_database()
    client_influx.create_database(DATABASE)
    client_influx.switch_database(DATABASE)

    while True:
        try:
            # Instanciar clase SubMQTT y suscribirse a los topics
            mqtt_client = SubMqtt(ip_address=BROKER, client_influx=client_influx)
            mqtt_client.subscribe_topics(topic=TOPIC, qos=QOS)
            mqtt_client.start_broker()
        except ConnectionError as e:
            line, data = fn.traceback_logging()
            logging.error(
                f"Se ha generado un error con la base de datos local InfluxDB: {e} en la linea {line} -> {data}"
            )
            # Conectar con la base de datos InfluxDB
            client_influx = connect_database()
            sleep(TIMEOUT)
        except RequestsConnectionError as e:
            line, data = fn.traceback_logging()
            logging.error(f"Error con el servidor docker [mosquitto]: {e}")
            sleep(TIMEOUT)
        except Exception as e:
            line, data = fn.traceback_logging()
            logging.error(
                f"No se ha podido tratar el siguiente error: {e} en la linea {line} -> {data}"
            )
            sleep(TIMEOUT)
