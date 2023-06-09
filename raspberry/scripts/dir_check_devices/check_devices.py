"""
Comprueba si los dispositivos especificados a traves de su IP se encuentra en la red y registra
el resultado en una base de datos InfluxDB
"""
import logging
import os
import sys
from time import sleep

import ping3
from influxdb import InfluxDBClient
from configparser import ConfigParser

# Agrega la ruta al PYTHONPATH
sys.path.append("/home/ctacoronte/Escritorio/GitHub/raspberry-micgridblue")
sys.path.append(os.environ.get("PATH_MGB"))

from func import microgridblue as fn


def check_devices(hostnames: list[str], timeout: int = 1) -> dict:
    """
    Comprueba la conectividad de una lista de dispositivos.

    :param hostnames: Lista de nombres de host o direcciones IP de los dispositivos a comprobar.
    :type hostnames: list[str]
    :param timeout: Tiempo máximo de espera en segundos para cada intento de conexión.
                    El valor por defecto es 1 segundo.
    :type timeout: int, optional
    :return: Diccionario que contiene el estado de conectividad de los dispositivos,
             donde las claves son los nombres de host o direcciones IP y los valores son 1 si hay conexión
             o 0 si no hay conexión.
    :rtype: dict
    """
    # Definir diccionario que va almacenar los datos
    state_dict = dict()

    # Bucle que comprueba conectividad para cada host
    for hostname in hostnames:
        if hostname != '':
            response = ping3.ping(dest_addr=hostname, timeout=timeout)
            # Si hay respuesta = hay conexion
            if response is not None:
                state_dict[hostname] = 1
            else:
                state_dict[hostname] = 0

    return state_dict


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
    # Configurar y crear el archivo logging
    fn.create_logging(os.path.basename(__file__))

    logging.info("¡Inicio de procedimiento!".center(100, "*"))

    # Cargar la configuración desde el archivo .conf
    config = ConfigParser()
    config.read(fn.search_path_file("main.conf"))

    # Obtener los valores de la sección [main/devices]
    INTERVAL = config.getint("main/devices", "interval", fallback=30)
    DATABASE = config.get("main/devices", "database", fallback='check_devices')
    TABLE = config.get("main/devices", "table", fallback='data_check_devices')
    HOSTNAMES = config.get("main/devices", "hostnames").split(",")
    TIMEOUT = config.getint("main/devices", "timeout", fallback=2)

    logging.info(
        f"""Variables especificadas:
        \t\t\t\thostnames: {HOSTNAMES},
        \t\t\t\tdatabase: {DATABASE},
        \t\t\t\ttable: {TABLE},
        \t\t\t\tinterval: {INTERVAL},
        \t\t\t\ttimeout: {TIMEOUT}"""
    )

    # Conectar con la base de datos local InfluxDB
    client_influx = connect_database()
    client_influx.create_database(DATABASE)
    client_influx.switch_database(DATABASE)

    while True:
        try:
            # Obtener el estado de conexion de los dispositivos
            values_points = check_devices(HOSTNAMES, TIMEOUT)

            # Almacenar los datos en la base de datos InfluxDB
            if values_points:

                points = [
                    {
                        "measurement": TABLE,
                        "fields" : values_points
                    }
                ]

                client_influx.write_points(points=points, time_precision='s')

                if 0 in points[0]['fields'].values():
                    logging.error(points)
                else:
                    logging.info(points)

            sleep(INTERVAL)
        except ConnectionError as e:
            line, data = fn.traceback_logging()
            logging.error(
                f"Se ha generado un error con la base de datos local InfluxDB: {e} en la linea {line} -> {data}"
            )
            # Conectar con la base de datos InfluxDB
            client_influx = connect_database()
        except Exception as e:
            line, data = fn.traceback_logging()
            logging.error(
                f"No se ha podido tratar el siguiente error: {e} en la linea {line} -> {data}"
            )
            sleep(INTERVAL)
