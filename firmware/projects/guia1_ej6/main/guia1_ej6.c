#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
// Definir la estructura para el pin GPIO
typedef struct {
    int pin;         /*!< GPIO pin number */
    io_t dir;        /*!< GPIO direction */
} gpioConf_t;

// Función ficticia para configurar un pin GPIO
void gpioConfig(const gpio_t pin, io_t dir) {
    printf("Configurando GPIO %d como %s\n", pin, (dir == GPIO_OUTPUT) ? "salida" : "entrada");
}

// Función ficticia para escribir en un pin GPIO
void gpioWrite(const gpio_t pin, int value) {
    printf("Escribiendo en GPIO %d: %d\n", pin, value);
}

// Mapeo de pines para los dígitos del display LCD
const gpio_t digitPins[] = {GPIO_19, GPIO_18, GPIO_9};

void mostrarDatoLCD(uint32_t dato, int numDigitos, gpioConf_t *vectorGPIOS, gpioConf_t *vectorDigitos) {
    // Configurar los GPIOs de los dígitos
    for (int i = 0; i < numDigitos; i++) {
        gpioConfig(vectorDigitos[i].pin, GPIO_OUTPUT);
    }

    // Mostrar el dato en el display LCD
    for (int i = 0; i < numDigitos; i++) {
        // Obtener el dígito correspondiente
        int digito = (dato >> (4 * i)) & 0xF;

        // Configurar los GPIOs del dígito
        gpioConfig(vectorGPIOS[digito].pin, GPIO_OUTPUT);

        // Activar el dígito en el display LCD
        gpioWrite(vectorDigitos[i].pin, 1); // Activa el pin

        // Desactivar los otros dígitos en el display LCD
        for (int j = 0; j < numDigitos; j++) {
            if (j != i) {
                gpioWrite(vectorDigitos[j].pin, 0); // Desactiva el pin
            }
        }

        // Esperar un tiempo para mostrar el dígito en el display LCD
        // (Aquí puedes agregar un retardo apropiado para tu hardware)
    }
}

int main(void){
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
        // Aquí van los otros pines GPIO para los dígitos 4 a 15
    };

    // Ejemplo de dato de 32 bits
    uint32_t dato = 0xABCD1234;

    // Mostrar el dato en el display LCD
    mostrarDatoLCD(dato, 3, vectorGPIOS, vectorDigitos);

    return 0;
}
