"""
Comprueba el estado del dispositivo, en este caso optimizado para la raspberry pi 4B,
midiendo consumo de cpu y ram, espacio libre y temperatura de la cpu. Finalmente, se
almacena la informacion en una base de datos InfluxDB
"""
import logging
import os
import sys
from configparser import ConfigParser
from time import sleep

from influxdb import InfluxDBClient
from requests.exceptions import ConnectionError as RequestsConnectionError

# Agrega la ruta al PATH
sys.path.append("/home/ctacoronte/Escritorio/GitHub/raspberry-micgridblue")
sys.path.append(os.environ.get("PATH_MGB"))

from func import microgridblue as fn


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

    # Obtener los valores de la sección [main/raspberry]
    INTERVAL = config.getint("main/raspberry", "interval", fallback=30)
    DATABASE = config.get("main/raspberry", "database", fallback="raspberry")
    TABLE = config.get("main/raspberry", "table", fallback="data_raspberry")
    TIMEOUT = config.getint("main/raspberry", "timeout", fallback=100)

    logging.info(
        f"""Variables especificadas:
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
            cpu, ram, disk, temp = fn.status_raspberry()

            # Crear diccionario de datos
            # Valores anomalos = -1
            points = [
                {
                    "measurement": TABLE,
                    "fields": {
                        "porcentaje_uso_cpu": cpu.get("percentage_cpu", -1),
                        "ram_ocupada": ram.get("busy_ram", -1),
                        "memoria_ocupada": disk.get("busy_disk", -1),
                        "temperatura_cpu": temp.get("temp_cpu", -1),
                    },
                }
            ]

            # Comprobar parametros criticos
            # Valores anomalos = 101
            warning_cpu = cpu.get("percentage_cpu", 101) > 50
            warning_temp = temp.get("temp_cpu", 101) > 60
            warning_ram = ram.get("percentage_busy_ram", 101) > 60
            if warning_cpu or warning_temp or warning_ram:
                points[0]["fields"]["alerta_hardware"] = 1
                logging.warning(points)
            else:
                points[0]["fields"]["alerta_hardware"] = 0
                logging.info(points)

            # Escribir en base de datos local
            client_influx.write_points(points=points, time_precision="s")

            sleep(INTERVAL)
        except RequestsConnectionError as e:
            line, data = fn.traceback_logging()
            logging.error(
                f"Se ha generado un error con la base de datos local InfluxDB: {e} en la linea {line} -> {data}"
            )
            # Volver a crear cliente InfluxDB
            client_influx = connect_database()
        except Exception as e:
            line, data = fn.traceback_logging()
            logging.error(
                f"No se ha podido tratar el siguiente error: {e} en la linea {line} -> {data}"
            )
