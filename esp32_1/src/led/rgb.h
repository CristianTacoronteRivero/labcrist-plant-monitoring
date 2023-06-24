#ifndef LED_RGB_H
#define LED_RGB_H
#include <Arduino.h>

/**
 * Enciende el LED RGB con los colores y la intensidad especificados.
 *
 * @param rojo Valor de intensidad del color rojo (0-255).
 * @param verde Valor de intensidad del color verde (0-255).
 * @param azul Valor de intensidad del color azul (0-255).
 * @param pinRojo Pin utilizado para el color rojo.
 * @param pinVerde Pin utilizado para el color verde.
 * @param pinAzul Pin utilizado para el color azul.
 */
void commandLED(int rojo, int verde, int azul, int pinRojo, int pinVerde, int pinAzul);

/**
 * Apaga el LED RGB.
 *
 * @param pinRojo Pin utilizado para el color rojo.
 * @param pinVerde Pin utilizado para el color verde.
 * @param pinAzul Pin utilizado para el color azul.
 */
void turnOffLED(int pinRojo, int pinVerde, int pinAzul);

#endif
