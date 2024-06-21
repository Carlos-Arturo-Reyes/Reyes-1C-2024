#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "ble_mcu.h"
#include "l9110.h"
#include "servo_sg90.h"
#include "lcditse0803.h"
#include "gpio_mcu.h"

// Definición de pines GPIO de los motores
#define MOTOR_LEFT_A_1A GPIO_NUM_6 //amarillo, azul
#define MOTOR_LEFT_A_1B GPIO_NUM_5
#define MOTOR_RIGHT_B_1A GPIO_NUM_2
#define MOTOR_RIGHT_B_1B GPIO_NUM_3 //amarillo , amarillo

// Definición de pines GPIO de los servos
#define SERVO_OUT_1 SERVO_0
#define SERVO_OUT_2 SERVO_1

#define BUFFER_SIZE 20
#define ADC_CHANNEL ADC1_CHANNEL_6  // GPIO34 en ESP32

static char buffer[BUFFER_SIZE];
static int buffer_index = 0;
static int8_t servo_angle_1 = 0; // Ángulo inicial del primer servo
static int8_t servo_angle_2 = 0; // Ángulo inicial del segundo servo
static bool button_state = false; // Estado actual del botón
static bool last_button_state = false; // Estado anterior del botón

void control_motors(bool left_forward, bool left_backward, bool right_forward, bool right_backward) {
    if (left_forward) {
        printf("Left Motor Forward\n");
        L9110SetSpeed(MOTOR_A, 100);
    } else if (left_backward) {
        printf("Left Motor Backward\n");
        L9110SetSpeed(MOTOR_A, -100);
    } else {
        printf("Left Motor Stop\n");
        L9110SetSpeed(MOTOR_A, 0);
    }

    if (right_forward) {
        printf("Right Motor Forward\n");
        L9110SetSpeed(MOTOR_B, 100);
    } else if (right_backward) {
        printf("Right Motor Backward\n");
        L9110SetSpeed(MOTOR_B, -100);
    } else {
        printf("Right Motor Stop\n");
        L9110SetSpeed(MOTOR_B, 0);
    }
}

void servo_control_task(void *pvParameter) {
    while (1) {
        // Actualizar el ángulo de los servos
        ServoMove(SERVO_OUT_1, servo_angle_1);
        ServoMove(SERVO_OUT_2, servo_angle_2);
        
        vTaskDelay(10 / portTICK_PERIOD_MS); // Ajuste para evitar un uso intensivo de la CPU
    }
}

void read_data(uint8_t *data, uint8_t len) {
    // Agrega los nuevos bytes al buffer
    for (int i = 0; i < len; i++) {
        buffer[buffer_index++] = data[i];
        if (buffer_index >= BUFFER_SIZE) {
            buffer_index = 0; // Evita desbordamiento del buffer
        }
    }

    // Control basado en botones
    if (strstr(buffer, "0")) control_motors(false, false, false, false); // paro 
    if (strstr(buffer, "1")) control_motors(true, false, true, false); // adelante
    if (strstr(buffer, "3")) control_motors(false, true, false, true); // atrás
    if (strstr(buffer, "4")) { // izquierda
        control_motors(false, true, true, false);
    }
    if (strstr(buffer, "2")) { // derecha
        control_motors(true, false, false, true);
    }

    // Control del botón y servos
    if (*data == 'B') { // Cambiar estado del botón cuando se recibe la señal 'B'
        button_state = !button_state;
    }

    // Restablece el buffer después de procesar
    if (buffer_index > 0) {
        buffer_index = 0;
    }
}

void init_adc(void) {
    // Configurar el ADC
    adc1_config_width(ADC_WIDTH_BIT_12);  // Ancho del ADC de 12 bits
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_0);  // Atenuación de 0 dB
}

uint16_t read_soil_moisture(void) {
    // Leer el valor del ADC y convertirlo a una escala de 0 a 99
    int adc_value = adc1_get_raw(ADC_CHANNEL);
    uint16_t moisture = (adc_value * 100) / 4095;  // Escalar el valor a 0-99
    return moisture;
}

void display_moisture_on_lcd(uint16_t moisture) {
    if (!LcdItsE0803Write(moisture)) {
        printf("Error al mostrar el valor en la pantalla LCD\n");
    }
}

void soil_moisture_task(void *pvParameter) {
    while (1) {
        // Leer la humedad del suelo
        uint16_t moisture = read_soil_moisture();

        // Mostrar la humedad en la pantalla LCD
        display_moisture_on_lcd(moisture);

        // Esperar 1 segundo antes de leer y mostrar el valor nuevamente
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main() {
    // Inicializa los pines GPIO de los motores como salidas
    gpio_set_direction(MOTOR_LEFT_A_1A, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_LEFT_A_1B, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_RIGHT_B_1A, GPIO_MODE_OUTPUT);
    gpio_set_direction(MOTOR_RIGHT_B_1B, GPIO_MODE_OUTPUT);

    // Inicializa el controlador L9110
    L9110Init(MOTOR_LEFT_A_1A, MOTOR_LEFT_A_1B, MOTOR_RIGHT_B_1A, MOTOR_RIGHT_B_1B);

    // Inicializar los servos
    ServoInit(SERVO_OUT_1, GPIO_1); 
    ServoInit(SERVO_OUT_2, GPIO_10); 

    // Inicializar la pantalla LCD
    if (!LcdItsE0803Init()) {
        printf("Error al inicializar la pantalla LCD\n");
        return;
    }

    // Inicializar el ADC
    init_adc();
    
    // Crear tarea para controlar los servos
    xTaskCreate(&servo_control_task, "servo_control_task", 2048, NULL, 5, NULL);

    // Crear tarea para la lectura y visualización de la humedad del suelo
    xTaskCreate(&soil_moisture_task, "soil_moisture_task", 2048, NULL, 5, NULL);

    // Configuración de Bluetooth
    ble_config_t ble_configuration = {
        "SEMBRADOR",
        (void (*)(uint8_t *, uint8_t))read_data
    };
    BleInit(&ble_configuration);

    while (1) {
        // Verificar si se ha presionado el botón
        if (button_state != last_button_state) {
            if (button_state) {
                // Cambiar la posición de los servos
                servo_angle_1 = 90;
                servo_angle_2 = -90;
            } else {
                // Regresar los servos a su posición original
                servo_angle_1 = 0;
                servo_angle_2 = 0;
            }
            last_button_state = button_state;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
