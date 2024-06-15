/**
 * @file guia1_ej5.c
 * 
 * 
 * @brief escribir una función que reciba como parámetro un dígito BCD
 *  y un vector de estructuras del tipo gpioConf_t.
 * 
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/16/2024 | Document creation		                         |
 *
 * @author Reyes Carlos (carlosarturoreyes69@gmail.com)
 * 
 */
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"

// Definir los valores LOW y HIGH
#ifndef LOW
#define LOW 0
#endif

#ifndef HIGH
#define HIGH 1
#endif

/**
 * @brief Estructura para representar un pin GPIO.
 */
typedef struct {
    gpio_t pin;     /*!< Número del pin GPIO */
    io_t dir;       /*!< Dirección del GPIO */
} gpio_pin_t;

/**
 * @brief Configura un pin GPIO.
 *
 * @param pin Puntero a la estructura que representa el pin GPIO.
 */
void gpioConfig(const gpio_pin_t *pin) {
    GPIOInit(pin->pin, pin->dir);
}

/**
 * @brief Escribe un valor en un pin GPIO.
 *
 * @param pin Puntero a la estructura que representa el pin GPIO.
 * @param value Valor a escribir en el pin (LOW o HIGH).
 */
void gpioWrite(const gpio_pin_t *pin, int value) {
    GPIOState(pin->pin, value == HIGH);
}

/**
 * @brief Estructura para representar la configuración de un pin GPIO.
 */
typedef struct {
    gpio_pin_t pin; /*!< Estructura del pin GPIO */
    io_t dir;       /*!< Dirección del GPIO */
} gpioConf_t;

/**
 * @brief Actualiza los GPIOs según el valor BCD ingresado.
 *
 * @param bcd Valor BCD a ser escrito en los pines GPIO.
 * @param vectorGPIOS Puntero al vector de configuraciones de GPIO.
 */
void actualizarGPIOSegunBCD(uint8_t bcd, gpioConf_t *vectorGPIOS) {
    // Configurar los GPIOs
    for (int i = 0; i < 4; i++) {
        gpioConfig(&vectorGPIOS[i].pin);
    }

    // Actualizar los GPIOs según el BCD ingresado
    for (int i = 0; i < 4; i++) {
        // Verificar el estado del bit correspondiente en el BCD
        uint8_t estadoBit = (bcd >> i) & 0x01;

        // Cambiar el estado del GPIO según el estado del bit
        gpioWrite(&vectorGPIOS[i].pin, estadoBit);
    }
}

/**
 * @brief Función principal de la aplicación.
 *
 * Configura y actualiza los GPIOs según un valor BCD predefinido.
 */
void app_main(void) {
    // Definir el vector de GPIOs
    gpioConf_t vectorGPIOS[4] = {
        {{GPIO_20, GPIO_OUTPUT}, GPIO_OUTPUT},
        {{GPIO_21, GPIO_OUTPUT}, GPIO_OUTPUT},
        {{GPIO_22, GPIO_OUTPUT}, GPIO_OUTPUT},
        {{GPIO_23, GPIO_OUTPUT}, GPIO_OUTPUT}
    };

    // Ejemplo: BCD ingresado
    uint8_t bcd = 0b1101;

    // Actualizar los GPIOs según el BCD ingresado
    actualizarGPIOSegunBCD(bcd, vectorGPIOS);
}
