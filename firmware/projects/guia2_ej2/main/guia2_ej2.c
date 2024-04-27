#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "gpio_mcu.h"
#include "timer_mcu.h"
#include "driver/gpio.h"

#define CONFIG_BLINK_PERIOD_LED_1 1000
#define CONFIG_BLINK_PERIOD_LED_2 1500
#define CONFIG_BLINK_PERIOD_LED_3 500

#define TRIGGER_GPIO GPIO_14
#define ECHO_GPIO GPIO_12
#define HOLD_BUTTON GPIO_4
#define MEASURE_BUTTON GPIO_15

static TaskHandle_t led1_task_handle = NULL;
static TaskHandle_t led2_task_handle = NULL;
static TaskHandle_t led3_task_handle = NULL;
static TaskHandle_t measurement_task_handle = NULL;

static bool hold_flag = false;

static SemaphoreHandle_t hold_button_semaphore = NULL;
static SemaphoreHandle_t measure_button_semaphore = NULL;

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
    char lcd_buffer[32];
    bool lcd_initialized = LcdItsE0803Init();
    if (!lcd_initialized) {
        printf("Error: No se pudo inicializar el LCD\n");
        vTaskDelete(NULL);
    }
    while (true) {
        if (xSemaphoreTake(hold_button_semaphore, portMAX_DELAY) == pdTRUE) {
            hold_flag = true;
            xSemaphoreGive(hold_button_semaphore);
        } else if (xSemaphoreTake(measure_button_semaphore, portMAX_DELAY) == pdTRUE) {
            hold_flag = false;
            distance_cm = HcSr04ReadDistanceInCentimeters();
            snprintf(lcd_buffer, sizeof(lcd_buffer), "Distancia: %lu cm", distance_cm);
            LcdItsE0803Write(0); // Clear the display
            LcdItsE0803Write(strtoul(lcd_buffer, NULL, 10)); // Display the new string
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
            xSemaphoreGive(measure_button_semaphore);
        } else {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

void IRAM_ATTR HoldButtonInterrupt(void *pvParameters) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(hold_button_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void IRAM_ATTR MeasureButtonInterrupt(void *pvParameters) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(measure_button_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void app_main(void) {
    HcSr04Init(TRIGGER_GPIO, ECHO_GPIO);
    LedsInit();
    GPIOInit(HOLD_BUTTON, GPIO_INPUT);
    GPIOInit(MEASURE_BUTTON, GPIO_INPUT);
    gpio_set_intr_type(HOLD_BUTTON, GPIO_INTR_POSEDGE);
    gpio_set_intr_type(MEASURE_BUTTON, GPIO_INTR_POSEDGE);
    gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
    gpio_isr_handler_add(HOLD_BUTTON, HoldButtonInterrupt, NULL);
    gpio_isr_handler_add(MEASURE_BUTTON, MeasureButtonInterrupt, NULL);
    hold_button_semaphore = xSemaphoreCreateBinary();
    measure_button_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(Led1Task, "LED_1_TASK", 1024, NULL, 5, &led1_task_handle);
    xTaskCreate(Led2Task, "LED_2_TASK", 1024, NULL, 5, &led2_task_handle);
    xTaskCreate(Led3Task, "LED_3_TASK", 1024, NULL, 5, &led3_task_handle);
    xTaskCreate(MeasurementTask, "MEASUREMENT_TASK", 2048, NULL, 5, &measurement_task_handle);
}