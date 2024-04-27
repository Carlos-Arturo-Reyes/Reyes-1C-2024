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

#define TRIGGER_GPIO 14
#define ECHO_GPIO 12
#define HOLD_BUTTON GPIO_NUM_0
#define MEASURE_BUTTON GPIO_NUM_1

static TaskHandle_t led1_task_handle = NULL;
static TaskHandle_t led2_task_handle = NULL;
static TaskHandle_t led3_task_handle = NULL;
static TaskHandle_t measurement_task_handle = NULL;

static bool hold_flag = false;

static void Led1Task(void *pvParameter) {
    while (true) {
        printf("LED_1 ON\n");
        LedOn(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
        printf("LED_1 OFF\n");
        LedOff(LED_1);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_1 / portTICK_PERIOD_MS);
    }
}

static void Led2Task(void *pvParameter) {
    while (true) {
        printf("LED_2 ON\n");
        LedOn(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_2 / portTICK_PERIOD_MS);
        printf("LED_2 OFF\n");
        LedOff(LED_2);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_2 / portTICK_PERIOD_MS);
    }
}

static void Led3Task(void *pvParameter) {
    while (true) {
        printf("LED_3 ON\n");
        LedOn(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_3 / portTICK_PERIOD_MS);
        printf("LED_3 OFF\n");
        LedOff(LED_3);
        vTaskDelay(CONFIG_BLINK_PERIOD_LED_3 / portTICK_PERIOD_MS);
    }
}

static void MeasurementTask(void *pvParameter) {
    uint32_t distance_cm = 0;
    char lcd_buffer[16];
    bool lcd_initialized = LcdItsE0803Init();
    if (!lcd_initialized) {
        printf("Error: No se pudo inicializar el LCD\n");
        vTaskDelete(NULL);
    }
    while (true) {
        if (GPIORead(HOLD_BUTTON) == 0) {
            hold_flag = true;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else if (GPIORead(MEASURE_BUTTON) == 0) {
            hold_flag = false;
            distance_cm = HcSr04ReadDistanceInCentimeters();
            sprintf(lcd_buffer, "Distancia: %u cm", distance_cm);
            LcdItsE0803Clear();
            LcdItsE0803Print(lcd_buffer, 0, 0);
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
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        } else {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void app_main(void) {
    HcSr04Init(TRIGGER_GPIO, ECHO_GPIO);
    LedInit(LED_1);
    LedInit(LED_2);
    LedInit(LED_3);
    GPIOInit(HOLD_BUTTON, GPIO_MODE_INPUT);
    GPIOInit(MEASURE_BUTTON, GPIO_MODE_INPUT);
    xTaskCreate(Led1Task, "LED_1_TASK", 1024, NULL, 5, &led1_task_handle);
    xTaskCreate(Led2Task, "LED_2_TASK", 1024, NULL, 5, &led2_task_handle);
    xTaskCreate(Led3Task, "LED_3_TASK", 1024, NULL, 5, &led3_task_handle);
    xTaskCreate(MeasurementTask, "MEASUREMENT_TASK", 2048, NULL, 5, &measurement_task_handle);
}