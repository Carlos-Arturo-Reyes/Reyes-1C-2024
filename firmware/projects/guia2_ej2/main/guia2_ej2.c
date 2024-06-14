/**
 * @file main.c
 * @brief Application main file for handling LEDs and measuring distance with HC-SR04.
 * 
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/16/2024 | Document creation		                         |
 * 
 *  @author Reyes Carlos (carlosarturoreyes69@gmail.com)
 */

/*==================[inclusions]=============================================*/
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
#include "switch.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1 1000 /**< Blink period for LED 1 in milliseconds */
#define CONFIG_BLINK_PERIOD_LED_2 1500 /**< Blink period for LED 2 in milliseconds */
#define CONFIG_BLINK_PERIOD_LED_3 500  /**< Blink period for LED 3 in milliseconds */

#define TRIGGER_GPIO GPIO_14 /**< GPIO for HC-SR04 trigger */
#define ECHO_GPIO GPIO_12    /**< GPIO for HC-SR04 echo */

/*==================[internal data declaration]==============================*/
static TaskHandle_t led1_task_handle = NULL; /**< Task handle for LED 1 task */
static TaskHandle_t led2_task_handle = NULL; /**< Task handle for LED 2 task */
static TaskHandle_t led3_task_handle = NULL; /**< Task handle for LED 3 task */
static TaskHandle_t measurement_task_handle = NULL; /**< Task handle for measurement task */

static bool hold_flag = false; /**< Flag to hold measurement */

static SemaphoreHandle_t hold_button_semaphore = NULL; /**< Semaphore for hold button */
static SemaphoreHandle_t measure_button_semaphore = NULL; /**< Semaphore for measure button */

/*==================[internal functions declaration]=========================*/
/**
 * @brief Task to blink LED 1.
 * @param pvParameter Task parameter.
 */
static void Led1Task(void *pvParameter);

/**
 * @brief Task to blink LED 2.
 * @param pvParameter Task parameter.
 */
static void Led2Task(void *pvParameter);

/**
 * @brief Task to blink LED 3.
 * @param pvParameter Task parameter.
 */
static void Led3Task(void *pvParameter);

/**
 * @brief Task to measure distance using HC-SR04 and update the LCD display.
 * @param pvParameter Task parameter.
 */
static void MeasurementTask(void *pvParameter);

/**
 * @brief Interrupt service routine for hold button.
 * @param pvParameters ISR parameter.
 */
void IRAM_ATTR HoldButtonInterrupt(void *pvParameters);

/**
 * @brief Interrupt service routine for measure button.
 * @param pvParameters ISR parameter.
 */
void IRAM_ATTR MeasureButtonInterrupt(void *pvParameters);

/*==================[internal functions definition]==========================*/
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

/*==================[external functions definition]==========================*/
void app_main(void) {
    HcSr04Init(TRIGGER_GPIO, ECHO_GPIO);
    LedsInit();
    SwitchesInit();
    hold_button_semaphore = xSemaphoreCreateBinary();
    measure_button_semaphore = xSemaphoreCreateBinary();
    SwitchActivInt(SWITCH_1, HoldButtonInterrupt, NULL);
    SwitchActivInt(SWITCH_2, MeasureButtonInterrupt, NULL);
    xTaskCreate(Led1Task, "LED_1_TASK", 1024, NULL, 5, &led1_task_handle);
    xTaskCreate(Led2Task, "LED_2_TASK", 1024, NULL, 5, &led2_task_handle);
    xTaskCreate(Led3Task, "LED_3_TASK", 1024, NULL, 5, &led3_task_handle);
    xTaskCreate(MeasurementTask, "MEASUREMENT_TASK", 2048, NULL, 5, &measurement_task_handle);
}
