#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN D4     // pin de datos del sensor DHT11 conectado al pin D3/GPIO0
#define DHTTYPE DHT11   // tipo de sensor DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // baudios en 9600 para que funcione correctamente el Serial Monitor
  pinMode(LED_BUILTIN, OUTPUT); // configuracion del led del nodemcu
  dht.begin(); // inicia el sensor DHT11
}

void loop() {
  delay(2000); // esperar 2 segundos entre lecturas
  digitalWrite(LED_BUILTIN, LOW);
  float temp = dht.readTemperature(); // lee la temperatura del sensor
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(LED_BUILTIN, LOW);
  float humi = dht.readHumidity(); // lee la humedad del sensor
  digitalWrite(LED_BUILTIN, HIGH);

  // imprime por el serial los datos
  Serial.print("Temperatura: ");
  Serial.print(temp);
  Serial.print("°C, Humedad: ");
  Serial.print(humi);
  Serial.println("%"); //
}