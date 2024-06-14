#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "gpio_mcu.h"

#define CONFIG_BLINK_PERIOD_LED_1 1000
#define CONFIG_BLINK_PERIOD_LED_2 1500
#define CONFIG_BLINK_PERIOD_LED_3 500

#define TRIGGER_GPIO GPIO_2
#define ECHO_GPIO GPIO_3
#define HOLD_BUTTON GPIO_4
#define MEASURE_BUTTON GPIO_15

static TaskHandle_t measurement_task_handle = NULL;
static TaskHandle_t hold_button_task_handle = NULL;
static TaskHandle_t measure_button_task_handle = NULL;

static bool hold_flag = false;

static void HoldButtonTask(void *pvParameter) {
    while (true) {
        if (GPIORead(HOLD_BUTTON) == false) {  // Assuming false means the button is pressed
            hold_flag = true;
            vTaskDelay(1000 / portTICK_PERIOD_MS);  // debounce delay
        } else {
            hold_flag = false;
            vTaskDelay(100 / portTICK_PERIOD_MS);  // check button state periodically
        }
    }
}

static void MeasureButtonTask(void *pvParameter) {
    while (true) {
        if (!hold_flag && GPIORead(MEASURE_BUTTON) == false) {  // Assuming false means the button is pressed
            xTaskNotifyGive(measurement_task_handle);
            vTaskDelay(1000 / portTICK_PERIOD_MS);  // debounce delay
        } else {
            vTaskDelay(100 / portTICK_PERIOD_MS);  // check button state periodically
        }
    }
}

static void MeasurementTask(void *pvParameter) {
    uint32_t distance_cm = 0;
    char lcd_buffer[32];  // Increased buffer size to avoid overflow
    bool lcd_initialized = LcdItsE0803Init();
    if (!lcd_initialized) {
        printf("Error: No se pudo inicializar el LCD\n");
        vTaskDelete(NULL);
    }

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // wait for notification from MeasureButtonTask

        distance_cm = HcSr04ReadDistanceInCentimeters();
        snprintf(lcd_buffer, sizeof(lcd_buffer), "Distancia: %lu cm", distance_cm);
        LcdItsE0803Write(0);  // Clear the display
        LcdItsE0803WriteStr(lcd_buffer);  // Display the new string

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

void app_main(void) {
    HcSr04Init(TRIGGER_GPIO, ECHO_GPIO);
    LedsInit();
    GPIOInit(HOLD_BUTTON, GPIO_INPUT);
    GPIOInit(MEASURE_BUTTON, GPIO_INPUT);

    xTaskCreate(HoldButtonTask, "HOLD_BUTTON_TASK", 1024, NULL, 5, &hold_button_task_handle);
    xTaskCreate(MeasureButtonTask, "MEASURE_BUTTON_TASK", 1024, NULL, 5, &measure_button_task_handle);
    xTaskCreate(MeasurementTask, "MEASUREMENT_TASK", 2048, NULL, 5, &measurement_task_handle);
}
