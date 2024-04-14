#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
    uint8_t teclas;
    bool switch_1_pressed = false;
    bool switch_2_pressed = false;

    LedsInit();
    SwitchesInit();

    while(1) {
        teclas  = SwitchesRead();

        // Verificar si se presionó la tecla 1
        if (teclas & SWITCH_1) {
            switch_1_pressed = true;
        } else {
            switch_1_pressed = false;
        }

        // Verificar si se presionó la tecla 2
        if (teclas & SWITCH_2) {
            switch_2_pressed = true;
        } else {
            switch_2_pressed = false;
        }

        // Titilar LED 1 si se mantiene presionada la tecla 1
        if (switch_1_pressed) {
            LedToggle(LED_1);
        }

        // Titilar LED 2 si se mantiene presionada la tecla 2
        if (switch_2_pressed) {
            LedToggle(LED_2);
        }

        // Titilar LED 3 si se presionan simultáneamente las teclas 1 y 2
        if (switch_1_pressed && switch_2_pressed) {
            LedToggle(LED_3);
        }

        // Esperar un tiempo antes de la próxima iteración
        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}
