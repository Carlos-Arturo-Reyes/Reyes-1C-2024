/**
 * @file guia2_ej1.c
 * 
 * 
 * @brief Control de LEDs y sensor de ultrasonido con botones para medir y mantener la medición.
 * 
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/16/2024 | Document creation		                         |
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
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "gpio_mcu.h"

#define CONFIG_BLINK_PERIOD_LED_1 1000 ///< Periodo de parpadeo para LED 1 en ms
#define CONFIG_BLINK_PERIOD_LED_2 1500 ///< Periodo de parpadeo para LED 2 en ms
#define CONFIG_BLINK_PERIOD_LED_3 500  ///< Periodo de parpadeo para LED 3 en ms

#define TRIGGER_GPIO GPIO_2 ///< GPIO para la señal de trigger del sensor de ultrasonido
#define ECHO_GPIO GPIO_3    ///< GPIO para la señal de echo del sensor de ultrasonido
#define HOLD_BUTTON GPIO_4  ///< GPIO para el botón de mantener medición
#define MEASURE_BUTTON GPIO_15 ///< GPIO para el botón de medir distancia

static TaskHandle_t measurement_task_handle = NULL;
static TaskHandle_t hold_button_task_handle = NULL;
static TaskHandle_t measure_button_task_handle = NULL;

static bool hold_flag = false;

/**
 * @brief Tarea para manejar el estado del botón de mantener medición.
 * 
 * Esta tarea lee el estado del botón de mantener medición y actualiza la 
 * bandera hold_flag en consecuencia.
 * 
 * @param pvParameter Parámetro para la tarea (no utilizado).
 */
static void HoldButtonTask(void *pvParameter) {
    while (true) {
        if (GPIORead(HOLD_BUTTON) == false) {  // Asumimos que false significa que el botón está presionado
            hold_flag = true;
            vTaskDelay(1000 / portTICK_PERIOD_MS);  // retardo para debounce
        } else {
            hold_flag = false;
            vTaskDelay(100 / portTICK_PERIOD_MS);  // revisar el estado del botón periódicamente
        }
    }
}

/**
 * @brief Tarea para manejar el estado del botón de medir distancia.
 * 
 * Esta tarea lee el estado del botón de medir distancia y notifica a la 
 * tarea de medición si el botón está presionado.
 * 
 * @param pvParameter Parámetro para la tarea (no utilizado).
 */
static void MeasureButtonTask(void *pvParameter) {
    while (true) {
        if (!hold_flag && GPIORead(MEASURE_BUTTON) == false) {  // Asumimos que false significa que el botón está presionado
            xTaskNotifyGive(measurement_task_handle);
            vTaskDelay(1000 / portTICK_PERIOD_MS);  // retardo para debounce
        } else {
            vTaskDelay(100 / portTICK_PERIOD_MS);  // revisar el estado del botón periódicamente
        }
    }
}

/**
 * @brief Tarea para medir la distancia usando un sensor de ultrasonido.
 * 
 * Esta tarea mide la distancia cuando es notificada por MeasureButtonTask
 * y muestra la distancia en un LCD. También controla los LEDs basándose
 * en la distancia medida.
 * 
 * @param pvParameter Parámetro para la tarea (no utilizado).
 */
static void MeasurementTask(void *pvParameter) {
    uint32_t distance_cm = 0;
    char lcd_buffer[32];  // Tamaño aumentado para evitar desbordamiento
    bool lcd_initialized = LcdItsE0803Init();
    if (!lcd_initialized) {
        printf("Error: No se pudo inicializar el LCD\n");
        vTaskDelete(NULL);
    }

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // esperar notificación de MeasureButtonTask

        distance_cm = HcSr04ReadDistanceInCentimeters();
        snprintf(lcd_buffer, sizeof(lcd_buffer), "Distancia: %lu cm", distance_cm);
        LcdItsE0803Write(0);  // Limpiar la pantalla

        for (int i = 0; lcd_buffer[i] != '\0'; i++) {
            LcdItsE0803Write(lcd_buffer[i]);  // Mostrar la nueva cadena carácter por carácter
        }

        if (distance_cm < 10) {
            LedOff(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
        } else if (distance_cm >= 10 && distance_cm < 20) {
            LedOn(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);
        } else if (distance_cm >= 20 && distance_cm < 30) {
            LedOn(LED_1);
            LedOn(LED_2);
            LedOff(LED_3);
        } else {
            LedOn(LED_1);
            LedOn(LED_2);
            LedOn(LED_3);
        }
    }
}

/**
 * @brief Función principal de la aplicación.
 * 
 * Esta función inicializa el sensor de ultrasonido, los LEDs, y los botones.
 * También crea las tareas necesarias para manejar los botones y la medición.
 */
void app_main(void) {
    HcSr04Init(TRIGGER_GPIO, ECHO_GPIO);
    LedsInit();
    GPIOInit(HOLD_BUTTON, GPIO_INPUT);
    GPIOInit(MEASURE_BUTTON, GPIO_INPUT);

    xTaskCreate(HoldButtonTask, "HOLD_BUTTON_TASK", 1024, NULL, 5, &hold_button_task_handle);
    xTaskCreate(MeasureButtonTask, "MEASURE_BUTTON_TASK", 1024, NULL, 5, &measure_button_task_handle);
    xTaskCreate(MeasurementTask, "MEASUREMENT_TASK", 2048, NULL, 5, &measurement_task_handle);
}
