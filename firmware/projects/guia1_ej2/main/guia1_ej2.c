/**
 * @file guia1_ej2.c
 * 
 * 
 * @brief control de led por teclas 
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
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"

/** @brief Periodo de parpadeo en milisegundos. */
#define CONFIG_BLINK_PERIOD 1000

/** @addtogroup Macros y definiciones
 *  @{
 */

/** @} */ // Fin de "Macros y definiciones"

/** @addtogroup Datos internos
 *  @{
 */

/** @} */ // Fin de "Datos internos"

/** @addtogroup Funciones internas
 *  @{
 */

/** @} */ // Fin de "Funciones internas"

/** @addtogroup Funciones externas
 *  @{
 */

/**
 * @brief Función principal del programa.
 *
 * Esta función inicializa los LEDs y los switches, y entra en un bucle infinito donde lee el estado de los switches
 * y realiza acciones en consecuencia, como alternar el estado de los LEDs.
 */
void app_main(void) {
    uint8_t teclas;
    bool switch_1_pressed = false;
    bool switch_2_pressed = false;

    LedsInit();
    SwitchesInit();

    while(1) {
        teclas  = SwitchesRead();

        /** @brief Verificar si se presionó la tecla 1 */
        if (teclas & SWITCH_1) {
            switch_1_pressed = true;
        } else {
            switch_1_pressed = false;
        }

        /** @brief Verificar si se presionó la tecla 2 */
        if (teclas & SWITCH_2) {
            switch_2_pressed = true;
        } else {
            switch_2_pressed = false;
        }

        /** @brief Titilar LED 1 si se mantiene presionada la tecla 1 */
        if (switch_1_pressed) {
            LedToggle(LED_1);
        }

        /** @brief Titilar LED 2 si se mantiene presionada la tecla 2 */
        if (switch_2_pressed) {
            LedToggle(LED_2);
        }

        /** @brief Titilar LED 3 si se presionan simultáneamente las teclas 1 y 2 */
        if (switch_1_pressed && switch_2_pressed) {
            LedToggle(LED_3);
        }

        /** @brief Esperar un tiempo antes de la próxima iteración */
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}

/** @} */ // Fin de "Funciones externas"
