#include "rgb.h"
#include <Arduino.h>

void commandLED(int rojo, int verde, int azul, int pinRojo, int pinVerde, int pinAzul) {
  analogWrite(pinRojo, rojo);
  analogWrite(pinVerde, verde);
  analogWrite(pinAzul, azul);
}

void turnOffLED(int pinRojo, int pinVerde, int pinAzul) {
  commandLED(0, 0, 0, pinRojo, pinVerde, pinAzul); // Usa constantes aquí como una alternativa
}
