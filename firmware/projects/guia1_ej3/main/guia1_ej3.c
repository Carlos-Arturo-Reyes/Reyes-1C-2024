/*==================[inclusions]=============================================*/
/** @file guia1_ej3.c
 *  @brief Control de LEDs según la configuración de la estructura `leds`.
 *
 *  Este archivo contiene la función principal del programa y la función
 *  para controlar los LEDs según los parámetros especificados.
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
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"

/*==================[macros and definitions]=================================*/
/** @brief Periodo de parpadeo en milisegundos. */
#define CONFIG_BLINK_PERIOD 1000

/** @brief Constante para encender el LED. */
#define ON 1

/** @brief Constante para apagar el LED. */
#define OFF 2

/** @brief Constante para alternar el estado del LED. */
#define TOGGLE 3

/**
 * @struct leds
 * @brief Estructura para configurar el control de LEDs.
 */
struct leds {
    uint8_t mode;      /**< Modo de operación: ON, OFF, TOGGLE */
    uint8_t n_led;     /**< Número de LED a controlar */
    uint8_t n_ciclos;  /**< Cantidad de ciclos de encendido/apagado */
    uint16_t periodo;  /**< Tiempo de cada ciclo en milisegundos */
};

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/**
 * @brief Función para controlar los LEDs según los parámetros de la estructura `leds`.
 *
 * @param led_ptr Puntero a una estructura `leds` que contiene los parámetros de configuración.
 */
void controlar_leds(struct leds *led_ptr) {
    switch (led_ptr->mode) {
        case ON:
            if (led_ptr->n_led == 1) {
                LedOn(LED_1);
            } else if (led_ptr->n_led == 2) {
                LedOn(LED_2);
            } else if (led_ptr->n_led == 3) {
                LedOn(LED_3);
            }
            break;
        case OFF:
            if (led_ptr->n_led == 1) {
                LedOff(LED_1);
            } else if (led_ptr->n_led == 2) {
                LedOff(LED_2);
            } else if (led_ptr->n_led == 3) {
                LedOff(LED_3);
            }
            break;
        case TOGGLE:
            for (int i = 0; i < led_ptr->n_ciclos; i++) {
                if (led_ptr->n_led == 1) {
                    LedToggle(LED_1);
                } else if (led_ptr->n_led == 2) {
                    LedToggle(LED_2);
                } else if (led_ptr->n_led == 3) {
                    LedToggle(LED_3);
                }
                vTaskDelay(led_ptr->periodo / portTICK_PERIOD_MS);
            }
            break;
        default:
            // Modo no válido, no hagas nada
            break;
    }
}

/*==================[external functions definition]==========================*/

/**
 * @brief Función principal del programa.
 *
 * Esta función inicializa los LEDs y crea una instancia de la estructura `leds` con
 * la configuración deseada. Luego llama a la función `controlar_leds` para aplicar la
 * configuración y controlar los LEDs.
 */
void app_main(void) {
    LedsInit();

    // Crear una instancia de la estructura leds
    struct leds my_leds = {
        .mode = TOGGLE,    // Modo de encendido inicial
        .n_led = 1,        // Número de LED a controlar
        .n_ciclos = 10,    // Cantidad de ciclos de encendido/apagado
        .periodo = 500     // Tiempo de cada ciclo (en milisegundos)
    };

    // Llamar a la función para controlar los LEDs
    controlar_leds(&my_leds); 
}
/*==================[end of file]============================================*/
