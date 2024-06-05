#include <stdio.h>
#include <esp_system.h>
#include <esp_spi_flash.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Define motor control pins
#define MOTOR_FORWARD_PIN 18
#define MOTOR_BACKWARD_PIN 19

void motor_task(void *pvParameter) {
    // Initialize motor control pins as outputs
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << MOTOR_FORWARD_PIN) | (1ULL << MOTOR_BACKWARD_PIN);
    gpio_config(&io_conf);

    while (1) {
        // Move motor forward
        gpio_set_level(MOTOR_FORWARD_PIN, 1);
        gpio_set_level(MOTOR_BACKWARD_PIN, 0);
        printf("Motor moving forward...\n");
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        // Move motor backward
        gpio_set_level(MOTOR_FORWARD_PIN, 0);
        gpio_set_level(MOTOR_BACKWARD_PIN, 1);
        printf("Motor moving backward...\n");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    //nvs_flash_init();
    xTaskCreate(&motor_task, "motor_task", 2048, NULL, 5, NULL);
}