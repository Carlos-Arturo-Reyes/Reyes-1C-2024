/**
 * @file main.c
 * @brief Main application file for ESP32 UART, LED, and HC-SR04 interaction.
 *
 * This file contains the main application code for controlling LEDs, measuring 
 * distances with HC-SR04, and communicating via UART.
 * 
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/16/2024 | Document creation		                         |
 * 
 *  @author Reyes Carlos (carlosarturoreyes69@gmail.com)
 * 
 * 
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> 
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
#include "uart_mcu.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_LED_1 1000 /**< Blink period for LED 1 in ms */
#define CONFIG_BLINK_PERIOD_LED_2 1500 /**< Blink period for LED 2 in ms */
#define CONFIG_BLINK_PERIOD_LED_3 500  /**< Blink period for LED 3 in ms */

#define TRIGGER_GPIO GPIO_14 /**< GPIO for HC-SR04 trigger */
#define ECHO_GPIO GPIO_12    /**< GPIO for HC-SR04 echo */
#define HOLD_BUTTON GPIO_4   /**< GPIO for hold button */
#define MEASURE_BUTTON GPIO_15 /**< GPIO for measure button */
#define UART_PORT UART_PC    /**< UART port */
#define UART_BAUDRATE 115200 /**< UART baud rate */

static TaskHandle_t led1_task_handle = NULL; /**< Handle for LED1 task */
static TaskHandle_t led2_task_handle = NULL; /**< Handle for LED2 task */
static TaskHandle_t led3_task_handle = NULL; /**< Handle for LED3 task */
static TaskHandle_t measurement_task_handle = NULL; /**< Handle for measurement task */
static TaskHandle_t uart_task_handle = NULL; /**< Handle for UART task */

static bool hold_flag = false; /**< Flag to hold measurement */

static SemaphoreHandle_t hold_button_semaphore = NULL; /**< Semaphore for hold button */
static SemaphoreHandle_t measure_button_semaphore = NULL; /**< Semaphore for measure button */

/*==================[function definitions]===================================*/

/**
 * @brief Task to handle LED1 blinking.
 * @param pvParameter Pointer to task parameters (not used)
 */
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

/**
 * @brief Task to handle LED2 blinking.
 * @param pvParameter Pointer to task parameters (not used)
 */
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

/**
 * @brief Task to handle LED3 blinking.
 * @param pvParameter Pointer to task parameters (not used)
 */
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

/**
 * @brief Task to handle distance measurement and display.
 * @param pvParameter Pointer to task parameters (not used)
 */
static void MeasurementTask(void *pvParameter) {
    uint32_t distance_cm = 0;
    char lcd_buffer[32];
    char uart_buffer[32];
    bool lcd_initialized = LcdItsE0803Init();
    if (!lcd_initialized) {
        printf("Error: No se pudo inicializar el LCD\n");
        vTaskDelete(NULL);
    }

    serial_config_t uart_config = {
        .port = UART_PORT,
        .baud_rate = UART_BAUDRATE,
        .func_p = UART_NO_INT,
        .param_p = NULL
    };
    UartInit(&uart_config);

    while (true) {
        if (xSemaphoreTake(hold_button_semaphore, portMAX_DELAY) == pdTRUE) {
            hold_flag = !hold_flag;
            xSemaphoreGive(hold_button_semaphore);
        } else if (xSemaphoreTake(measure_button_semaphore, portMAX_DELAY) == pdTRUE) {
            if (!hold_flag) {
                distance_cm = HcSr04ReadDistanceInCentimeters();
                snprintf(lcd_buffer, sizeof(lcd_buffer), "Distancia: %lu cm", distance_cm);
                LcdItsE0803Write(0); // Clear the display
                LcdItsE0803Write(lcd_buffer); // Display the new string
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

                snprintf(uart_buffer, sizeof(uart_buffer), "%03lu cm\r\n", distance_cm);
                UartSendString(UART_PORT, uart_buffer);
            }
            xSemaphoreGive(measure_button_semaphore);
        } else {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
}

/**
 * @brief Task to handle UART communication.
 * @param pvParameter Pointer to task parameters (not used)
 */
static void UartTask(void *pvParameter) {
    uint8_t data;
    while (true) {
        if (UartReadByte(UART_PORT, &data)) {
            if (data == 'H') {
                hold_flag = true;
            } else if (data == 'R') {
                hold_flag = false;
            } else if (data == 'S') {
                LedOff(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

/**
 * @brief Interrupt handler for hold button.
 * @param pvParameters Pointer to parameters (not used)
 */
void IRAM_ATTR HoldButtonInterrupt(void *pvParameters) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(hold_button_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Interrupt handler for measure button.
 * @param pvParameters Pointer to parameters (not used)
 */
void IRAM_ATTR MeasureButtonInterrupt(void *pvParameters) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(measure_button_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Main application entry point.
 */
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
    xTaskCreate(UartTask, "UART_TASK", 1024, NULL, 5, &uart_task_handle);
}
