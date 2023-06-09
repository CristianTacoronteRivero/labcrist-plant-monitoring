"""Orquestador principal de ficheros python y funciones Linux
"""
import logging
import multiprocessing
import os
import subprocess
import sys
import time
from configparser import ConfigParser

from apscheduler.schedulers.background import BackgroundScheduler
from apscheduler.triggers.cron import CronTrigger

# obtiene el path por defecto de la raspberry
path_main = os.path.dirname(os.path.abspath(__file__))

# Establece las variables de entorno
sys.path.append('/home/ctacoronte/Escritorio/GitHub/raspberry-micgridblue')
os.environ["PATH_MGB"] = path_main

# Agrega la ruta al PATH
sys.path.append(os.environ.get("PATH_MGB"))

from func import microgridblue as fn

# Declaracion de funciones
def check_async_processes(process: None, path: str, asynchronous: bool = True) -> None:
    """Comprueba si el proceso se ha ejecutado correcyamente o no.
    Si el proceso es asincrono, comprobamos si se ha ejecutado en
    segundo plano, sino no ya que los procesos sincronos se quedan
    a la escicha de forma infinita.

    :param process: Resultado del proceso ejecutado
    :type process: None
    :param path: Ruta del archivo que se ha ejecutado en el proceso
    :type path: str
    :param asynchronous: True si es asincrono el proceso, por defecto es True
    :type asynchronous: bool, opcional
    """
    try:
        if process.poll() is None and asynchronous:
            logging.info(f"Fichero {path} ejecutado correctamente")
        else:
            logging.error(f"Error al ejecutar el fichero {path}")
    except AttributeError:
        if process == 0:
            logging.info(f"Ejecucion del fichero {path} finalizado correctamente")
        else:
            logging.error(f"Error al ejecutar el fichero {path}")

def run_check_raspberry() -> None:
    path = fn.search_path_file("check_raspberry.py")
    command = ["python3", path]
    process = subprocess.Popen(command)
    if isinstance(process.poll(), type(None)):
        logging.info(f"Proceso run_check_devices lanzado con PID = {process.pid}.")
    else:
        logging.info(f"Proceso run_check_devices lanzado con PID = {process.pid}. ¡ERROR!")

def run_mqtt_sub() -> None:
    path = fn.search_path_file("mqtt_sub.py")
    command = ["python3", path]
    process = subprocess.Popen(command)
    if isinstance(process.poll(), type(None)):
        logging.info(f"Proceso run_mqtt_sub lanzado con PID = {process.pid}.")
    else:
        logging.info(f"Proceso run_mqtt_sub lanzado con PID = {process.pid}. ¡ERROR!")

def run_check_devices() -> None:
    path = fn.search_path_file("check_devices.py")
    command = ["python3", path]
    process = subprocess.Popen(command)
    if isinstance(process.poll(), type(None)):
        logging.info(f"Proceso run_check_devices lanzado con PID = {process.pid}.")
    else:
        logging.info(f"Proceso run_check_devices lanzado con PID = {process.pid}. ¡ERROR!")

def run_esios() -> None:
    path = fn.search_path_file("esios.py")
    process = subprocess.call(["python3", path])
    logging.info(f"Resultado del proceso de run_esios: {process}")


# Lanzamiento del script
if __name__ == "__main__":
    # Crear logger
    fn.create_logging(os.path.basename(__file__))

    fn.logging.info("Script principal ejecutado".center(100, "*"))

    # Cargar la configuración desde el archivo .conf
    config = ConfigParser()
    config.read(fn.search_path_file("main.conf"))

    # Obtener el valor del bucle while
    path_esios = fn.search_path_file("esios.py")
    time_sleep = config.getint("MAIN", "time_sleep")

    ###############################################
    #            PROCESOS ASINCRONOS              #
    ###############################################

    check_raspberry = multiprocessing.Process(target=run_check_raspberry)

    check_devices = multiprocessing.Process(target=run_check_devices)

    # shelly_1 = multiprocessing.Process(target=run_mqtt_sub)

    # lanzo los procesos asincronos
    check_raspberry.start()
    check_devices.start()
    # shelly_1.start()


    ###############################################
    #             PROCESOS SINCRONOS              #
    ###############################################

    # Crear scheduler
    scheduler = BackgroundScheduler()

    # Ejecutar fichero esios cada dia a las 23:00 de la noche
    trigger_esios = CronTrigger(hour='21,22,23')
    esios = scheduler.add_job(func=run_esios, trigger=trigger_esios)

    scheduler.start()

    while True:
        time.sleep(time_sleep)
