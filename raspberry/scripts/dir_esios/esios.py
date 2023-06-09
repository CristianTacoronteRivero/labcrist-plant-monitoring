"""
Realiza una o mas consultas a la plataforma ESios para descargar el PVPC y
el precio del excedente de generacion en Canarias y guardarlo en InfluxDB
"""
import logging
import os
import sys
from configparser import ConfigParser
from datetime import datetime, timedelta
from time import sleep
from typing import Optional

import pandas as pd
import requests
from dateutil import parser
from influxdb import InfluxDBClient
from requests.exceptions import ConnectionError as RequestsConnectionError

# Agrega la ruta al PYTHONPATH
sys.path.append("/home/ctacoronte/Escritorio/GitHub/raspberry-micgridblue")
sys.path.append(os.environ.get("PATH_MGB"))

from func import microgridblue as fn


def normalize_datetime(date_time: str) -> str:
    """Intuye el formato de fecha introducido y lo normaliza
    al formato %d-%m-%Y para que no haya problema con la API de
    esios

    :param date_time: Fecha que se quiere noralizar
    :type date_time: str
    :return: Fecha normalizada
    :rtype: str
    """
    return parser.parse(date_time).strftime("%d-%m-%Y")


def next_day(days: int = 1) -> str:
    """Obtiene la fecha de n dias siguientes para obtener los
    datos de la plataforma esios.

    :param days: numero de dia posterior que se quiere obtener
     respecto al momento de ejecucion de la funcion
    :type days: int
    :return: fecha n dias posteriores en formato "%d-%m-%Y"
    :rtype: str
    """
    next_date = datetime.now() + timedelta(days=days)
    return next_date.strftime("%d-%m-%Y")


def yesterday(days: int = 1) -> str:
    """Obtiene la fecha de n dias anteriores para obtener los
    datos de la plataforma esios.

    :param days: numero de dia posterior que se quiere obtener
     respecto al momento de ejecucion de la funcion
    :type days: int
    :return: fecha n dias posteriores en formato "%d-%m-%Y"
    :rtype: str
    """
    next_date = datetime.now() - timedelta(days=days)
    return next_date.strftime("%d-%m-%Y")


def download_data(
    esios_token: str,
    indicator: list[int],
    start_date: Optional[str] = None,
    end_date: Optional[str] = None,
) -> pd.DataFrame:
    """
    Descarga los datos de indicadores desde la API de esios.ree.es y los devuelve como un DataFrame de pandas.

    :param esios_token: Token de autenticación para acceder a la API de esios.ree.es.
    :type esios_token: str
    :param indicator: Lista de identificadores de indicadores para los cuales se descargarán los datos.
    :type indicator: list[int]
    :param start_date: Fecha de inicio en formato "YYYY-MM-DD" para el rango de datos a descargar. Si no se especifica, se utilizará la fecha actual.
    :type start_date: Optional[str]
    :param end_date: Fecha de finalización en formato "YYYY-MM-DD" para el rango de datos a descargar. Si no se especifica, se utilizará la fecha actual.
    :type end_date: Optional[str]
    :return: DataFrame que contiene los datos descargados de los indicadores especificados.
    :rtype: pd.DataFrame
    """
    # Definir end point
    end_point = "https://api.esios.ree.es/indicators"

    # Definir encabezado
    headers = {
        "Accept": "application/json; application/vnd.esios-api-v2+json",
        "Content-Type": "application/json",
        "Host": "api.esios.ree.es",
        "Cookie": "",
        "x-api-key": esios_token,
        "Cache-Control": "no-cache",
        "Pragma": "no-cache",
    }

    # Especificar rango de fechas si no se ha introducido
    if isinstance(start_date, type(None)) and isinstance(end_date, type(None)):
        start_date = next_day()
        end_date = next_day()
    else:
        # Casuisticas para futuras versiones
        pass

    # Crear  lista de puntos para registrar en InfluxDB
    points = []

    for ind in indicator:
        # construir estrcutura de la API
        url = f"{end_point}/{ind}?start_date={start_date}T00:00&end_date={end_date}T23:59&group_by=hour"
        logging.info(f"Url generada: {url}")

        # Realizar la consulta
        response_requests = requests.get(url, headers=headers, timeout=TIMEOUT)
        successful_connection = 200
        if response_requests.status_code == successful_connection:
            logging.info(
                f"Requests satisfactorio: {response_requests} para el indicador {ind}"
            )
        else:
            logging.error(
                f"Requests no satisfactorio: {response_requests} para el indicador {ind}"
            )

        # Almacenar json
        response = response_requests.json().get("indicator")

        # Extraer nombre parametro
        name_indicator = {1739: "mwh_pvpc_venta_excedente", 1001: "mwh_pvpc"}
        measurement = name_indicator.get(ind)

        # Extraer los datos de valores del diccionario
        values = response["values"]

        if values:
            for item in values:
                # Crear filtro
                # 1. Para venta_excedente solo se quiere coger España
                # 2. Para PVPC solo se quiere coger Canarias
                geo_id_spain = (
                    item["geo_id"] == 3 and measurement == "mw/h_pvpc_venta_excedente"
                )
                geo_id_canarias = item["geo_name"].lower() == "canarias"
                if geo_id_spain or geo_id_canarias:
                    point = {
                        "measurement": measurement,
                        "tags": {"geo_name": item["geo_name"]},
                        "time": item["datetime_utc"],
                        "fields": {"value": item["value"]},
                    }
                    points.append(point)

    return points


def replace_timezone(date_time: str) -> datetime:
    """Elimina time zone de los objetos datetime
    para asi poder usarlo como index en una base
    de datos InfluxDB.

    :param date_time: Fecha la cual se desea eliminar la zona horaria
    :type date_time: str
    :return: Fecha sin zona horaria
    :rtype: datetime
    """
    dt_zone = parser.parse(date_time)
    return dt_zone.replace(tzinfo=None)


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
                "Establecimiento de conexión satisfactoria con la base de datos"
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
    # configura y crea archivo logging
    fn.create_logging(os.path.basename(__file__))

    fn.logging.info("Script principal ejecutado".center(100, "*"))

    # Cargar la configuración desde el archivo .conf
    config = ConfigParser()
    config.read(fn.search_path_file("main.conf"))

    TIMEOUT = config.getint("ESIOS", "timeout")
    DATABASE = config.get("ESIOS", "database")
    TABLE = config.get("ESIOS", "table")
    TOKEN = config.get("ESIOS", "token")

    # 1739: precio excedente
    # 1001: pvpc
    indicador = [1739, 1001]
    logging.info("Indicadores a descargar: {1739: precio excedente}, {1001: pvpc}")

    # Conectar con la base de datos local InfluxDB
    client_influx = connect_database()
    client_influx.create_database(DATABASE)
    client_influx.switch_database(DATABASE)

    try:
        # Obtener datos en una primera instancia
        values_indicators = download_data(esios_token=TOKEN, indicator=indicador)

        # En el caso de que la API aun no haya sacado los datos lanza un error
        if len(values_indicators) == 0:
            logging.error("Datos no disponibles")

            raise ValueError("Datos no disponibles en el momento de ejecución.")

        # Registrar datos en base de datos InfluxDB
        client_influx.write_points(points=values_indicators, time_precision="s")
        logging.info(values_indicators)


    except RequestsConnectionError as e:
        line, data = fn.traceback_logging()
        logging.error(
            f"Se ha generado un ERROR con la base de datos local InfluxDB: {e} en la linea {line} -> {data}"
        )
    except ValueError as e:
        line, data = fn.traceback_logging()
        logging.error(
            f"Se ha generado un ERROR con el acceso a los datos: {e} en la linea {line} -> {data}"
        )
    except Exception as e:
        line, data = fn.traceback_logging()
        logging.error(
            f"No se ha podido tratar el siguiente ERROR: {e} en la linea {line} -> {data}"
        )
    finally:
        client_influx.close()
