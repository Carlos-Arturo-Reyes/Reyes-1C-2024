/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
#define ON 1
#define OFF 2
#define TOGGLE 3

// Definición de la estructura leds
struct leds {
    uint8_t mode;      // ON, OFF, TOGGLE
    uint8_t n_led;     // indica el número de led a controlar
    uint8_t n_ciclos;  // indica la cantidad de ciclos de encendido/apagado
    uint16_t periodo;  // indica el tiempo de cada ciclo
};

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

// Función para controlar los LEDs según los parámetros de la estructura leds
void controlar_leds(struct leds *led_ptr) {
    for (int i = 0; i < led_ptr->n_ciclos; i++) {
        switch (led_ptr->mode) {
            case ON:
                LedOn(led_ptr->n_led);
                break;
            case OFF:
                LedOff(led_ptr->n_led);
                break;
            case TOGGLE:
                LedToggle(led_ptr->n_led);
                break;
            default:
                // Modo no válido, no hagas nada
                break;
        }
        vTaskDelay(led_ptr->periodo / portTICK_PERIOD_MS);
        // Invierte el modo para el siguiente ciclo
        if (led_ptr->mode == ON) {
            led_ptr->mode = OFF;
        } else {
            led_ptr->mode = ON;
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    LedsInit();

    // Crear una instancia de la estructura leds
    struct leds my_leds = {
        .mode = ON,        // Modo de encendido inicial
        .n_led = LED_1,    // Número de LED a controlar
        .n_ciclos = 10,    // Cantidad de ciclos de encendido/apagado
        .periodo = 500     // Tiempo de cada ciclo (en milisegundos)
    };

    // Llamar a la función para controlar los LEDs
    controlar_leds(&my_leds); 
}
/*==================[end of file]============================================*/


