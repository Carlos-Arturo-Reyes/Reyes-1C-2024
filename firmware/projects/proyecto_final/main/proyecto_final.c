#include <stdio.h>
#include <stdint.h>
#include "servo_sg90.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*Definición de la configuración del servo */
#define SERVO_GPIO 16
#define SERVO_NUM SERVO_0
#define SERVO2_NUM SERVO_1
#define SERVO2_GPIO 17

/* Función de callback para el servo */
void servo_callback(servo_out_t servo, int8_t angle)
{
    printf("Servo %d moved to angle %d\n", servo, angle);
}

/* Función principal */
void app_main(void)
{
    /* Inicialización del servo */
    uint8_t ret = ServoInit(SERVO_NUM, SERVO_GPIO);
    if (ret != 0)
    {
        printf("Error initializing servo: %u\n", ret);
        return;
    }

    /* Inicialización del segundo servo */
    uint8_t ret2 = ServoInit(SERVO2_NUM, SERVO2_GPIO);
    if (ret2 != 0)
    {
        printf("Error initializing servo2: %u\n", ret2);
        return;
    }

    /* Movimiento del servo */
    ServoMove(SERVO_NUM, 45);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ServoMove(SERVO_NUM, -45);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ServoMove(SERVO_NUM, 0);

    /* Movimiento del segundo servo */
    ServoMove(SERVO2_NUM, 90);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ServoMove(SERVO2_NUM, -90);
    vTaskDelay(pdMS_TO_TICKS(1000));
    ServoMove(SERVO2_NUM, 0);

    /* Registro de una función de callback para el servo */
    //ServoRegisterCallback(SERVO_NUM, servo_callback);
}