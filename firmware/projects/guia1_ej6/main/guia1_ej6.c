/**
 * @file guia1_ej6.c
 * 
 * 
 * @brief Escriba una función que reciba un dato de 32 bits,  la cantidad de dígitos de salida y 
 * dos vectores de estructuras del tipo  gpioConf_t. Uno  de estos vectores es igual al definido 
 * en el ejercicio 5 y el otro vector mapea los puertos con el dígito del LCD a donde mostrar un dato:
 *Dígito 1 -> GPIO_19
 *Dígito 2 -> GPIO_18
 *Dígito 3 -> GPIO_9
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

/**
 * @brief Estructura para el pin GPIO.
 */
typedef struct {
    int pin;         /*!< Número de pin GPIO */
    io_t dir;        /*!< Dirección del GPIO */
} gpioConf_t;

/**
 * @brief Mapeo de pines para los dígitos del display LCD.
 */
const gpio_t digitPins[] = {GPIO_19, GPIO_18, GPIO_9};

/**
 * @brief Muestra un dato en el display LCD.
 *
 * Esta función configura los GPIOs necesarios y muestra un dato en el display LCD.
 *
 * @param dato El dato de 32 bits a mostrar.
 * @param numDigitos Número de dígitos del display LCD.
 * @param vectorGPIOS Vector de configuración de los GPIOs para el display LCD.
 * @param vectorDigitos Vector de configuración de los GPIOs para los dígitos del display LCD.
 */
void mostrarDatoLCD(uint32_t dato, int numDigitos, gpioConf_t *vectorGPIOS, gpioConf_t *vectorDigitos) {
    // Configurar los GPIOs de los dígitos
    for (int i = 0; i < numDigitos; i++) {
        GPIOInit(vectorDigitos[i].pin, GPIO_OUTPUT);
    }

    // Mostrar el dato en el display LCD
    for (int i = 0; i < numDigitos; i++) {
        // Obtener el dígito correspondiente
        int digito = (dato >> (4 * i)) & 0xF;

        // Configurar los GPIOs del dígito
        GPIOInit(vectorGPIOS[digito].pin, GPIO_OUTPUT);

        // Activar el dígito en el display LCD
        GPIOOn(vectorDigitos[i].pin); // Activa el pin

        // Desactivar los otros dígitos en el display LCD
        for (int j = 0; j < numDigitos; j++) {
            if (j != i) {
                GPIOOff(vectorDigitos[j].pin); // Desactiva el pin
            }
        }

        // Esperar un tiempo para mostrar el dígito en el display LCD
        // (Aquí puedes agregar un retardo apropiado para tu hardware)
    }
}

/**
 * @brief Función principal de la aplicación.
 *
 * Esta función configura los GPIOs y muestra un dato en el display LCD.
 */
void app_main(void) {
    // Definir el vector de GPIOs para los dígitos del display LCD
    gpioConf_t vectorDigitos[] = {
        {GPIO_19, GPIO_OUTPUT},
        {GPIO_18, GPIO_OUTPUT},
        {GPIO_9, GPIO_OUTPUT}
    };

    // Definir el vector de GPIOs para el display LCD
    gpioConf_t vectorGPIOS[16] = {
        {GPIO_20, GPIO_OUTPUT},
        {GPIO_21, GPIO_OUTPUT},
        {GPIO_22, GPIO_OUTPUT},
        {GPIO_23, GPIO_OUTPUT},

    };

    // Ejemplo de dato de 32 bits
    uint32_t dato = 0xABCD1234;

    // Mostrar el dato en el display LCD
    mostrarDatoLCD(dato, 3, vectorGPIOS, vectorDigitos);
}
