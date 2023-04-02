#include <SPI.h> // biblioteca SPI
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 4 // pin de datos del sensor DHT11 conectado al pin 4 (MISO)
#define DHTTYPE DHT11 // tipo de sensor DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // baudios en 9600 para que funcione correctamente el Serial Monitor
  pinMode(LED_BUILTIN, OUTPUT); // configuracion del led del nodemcu
  dht.begin(); // inicia el sensor DHT11
  SPI.begin(); // inicia la comunicación SPI
}

void loop() {
  delay(2000); // esperar 2 segundos entre lecturas
  digitalWrite(LED_BUILTIN, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // establece la velocidad y modo de SPI
  float temp = dht.readTemperature(); // lee la temperatura del sensor
  SPI.endTransaction(); // finaliza la comunicación SPI
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(LED_BUILTIN, LOW);
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // establece la velocidad y modo de SPI
  float humi = dht.readHumidity(); // lee la humedad del sensor
  SPI.endTransaction(); // finaliza la comunicación SPI
  digitalWrite(LED_BUILTIN, HIGH);

  // imprime por el serial los datos
  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.print("°C, Humedad: ");
  Serial.print(humi);
  Serial.println("%"); //
}
