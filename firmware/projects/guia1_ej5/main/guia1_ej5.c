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

// Definir la enumeración para la dirección de GPIO
typedef enum {
    GPIO_INPUT,
    GPIO_OUTPUT
} gpio_dir_t;

// Definir la estructura para el pin GPIO
typedef struct {
    int pin;         /*!< GPIO pin number */
    gpio_dir_t dir;  /*!< GPIO direction */
} gpio_pin_t;

// Función ficticia para configurar un pin GPIO
void gpioConfig(const gpio_pin_t *pin) {
    printf("Configurando GPIO %d como %s\n", pin->pin, (pin->dir == GPIO_OUTPUT) ? "salida" : "entrada");
}

// Función ficticia para escribir en un pin GPIO
void gpioWrite(const gpio_pin_t *pin, int value) {
    printf("Escribiendo en GPIO %d: %d\n", pin->pin, value);
}

typedef struct {
    gpio_pin_t pin;         /*!< GPIO pin */
    gpio_dir_t dir;         /*!< GPIO direction */
} gpioConf_t;

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

int main(void){
    // Definir el vector de GPIOs
    gpioConf_t vectorGPIOS[4] = {
        {{20, GPIO_OUTPUT}, GPIO_OUTPUT},
        {{21, GPIO_OUTPUT}, GPIO_OUTPUT},
        {{22, GPIO_OUTPUT}, GPIO_OUTPUT},
        {{23, GPIO_OUTPUT}, GPIO_OUTPUT}
    };

    // Ejemplo: BCD ingresado
    uint8_t bcd = 0b1101;

    // Actualizar los GPIOs según el BCD ingresado
    actualizarGPIOSegunBCD(bcd, vectorGPIOS);

    return 0;
}
