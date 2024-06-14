/**
 * @file examen.c
 * @brief Control de riego y pH para una planta usando ESP32
 *  * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 *
 * @author Carlos Arturo Reyes 
 * Este código controla el suministro de agua y el pH de una planta utilizando
 * una ESP32, varios sensores y bombas peristálticas.
 * 
 * 
 * 
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h" // configura y usa el adc en modo de un disparo 
#include "esp_adc/adc_cali.h" // proporcion acalibracion a mi adc 
#include "esp_adc/adc_cali_scheme.h" //defines los esquemas de calibracion del adc
//Quiero hacer una aclaracion de estas librerias son unas que yo ya habia usado en un proyecto similar 
// anterior en mi otra universidad 
//use las librerias para las conversiones de adc 

/*==================[macros and definitions]=================================*/
#define SENSOR_HUMEDAD_GPIO    GPIO_NUM_4  /**< GPIO del sensor de humedad */
#define BOMBA_AGUA_GPIO        GPIO_NUM_16 /**< GPIO de la bomba de agua */
#define BOMBA_ACIDA_GPIO       GPIO_NUM_17 /**< GPIO de la bomba de solución ácida */
#define BOMBA_BASICA_GPIO      GPIO_NUM_18 /**< GPIO de la bomba de solución básica */

#define ADC1_CHANNEL_PH        ADC_CHANNEL_6 /**< Canal ADC para el sensor de pH (GPIO34) */

#define PH_MIN                 6.0 /**< pH mínimo aceptable */
#define PH_MAX                 6.7 /**< pH máximo aceptable */

/*==================[internal data definition]===============================*/
static adc_oneshot_unit_handle_t adc1_handle; /**< Handle para la unidad ADC */
static adc_cali_handle_t adc1_cali_handle; /**< Handle para la calibración del ADC */

/*==================[internal functions declaration]=========================*/
/**
 * @brief Inicializa el ADC y la calibración
 */
static void init_adc();

/**
 * @brief Lee el valor de pH desde el sensor
 * 
 * @return El valor de pH leído
 */
static float read_ph();

/**
 * @brief Controla el riego y el pH de la planta
 * 
 * @param arg Argumento pasado a la tarea (no se usa)
 */
static void control_riego_y_ph(void *arg);

/*==================[external functions definition]==========================*/
/**
 * @brief Función principal del programa
 */


void app_main(void) {
    // Inicializar el ADC para el sensor de pH
    init_adc();
    
    // Configurar los GPIOs
    gpio_set_direction(SENSOR_HUMEDAD_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(BOMBA_AGUA_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BOMBA_ACIDA_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(BOMBA_BASICA_GPIO, GPIO_MODE_OUTPUT);

    // Crear la tarea de control de riego y pH, esta es la principal tarea 
    xTaskCreate(control_riego_y_ph, "control_riego_y_ph", 2048, NULL, 5, NULL);
}



/*==================[internal functions definition]=========================*/
static void init_adc() {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    adc_oneshot_new_unit(&init_config, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_0,
    };
    adc_oneshot_config_channel(adc1_handle, ADC1_CHANNEL_PH, &config);

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_0,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_cali_create_scheme_curve_fitting(&cali_config, &adc1_cali_handle);
}

static float read_ph() {
    int raw_value;
    adc_oneshot_read(adc1_handle, ADC1_CHANNEL_PH, &raw_value);
    int voltage;
    adc_cali_raw_to_voltage(adc1_cali_handle, raw_value, &voltage);
    float ph = (voltage / 1000.0) * 14.0 / 3.0; // Ajustar la fórmula según la calibración
    return ph;
}

static void control_riego_y_ph(void *arg) {
    bool bomba_agua_encendida = false;
    bool bomba_acida_encendida = false;
    bool bomba_basica_encendida = false;

    while (1) {
        // Leer humedad
        int humedad = gpio_get_level(SENSOR_HUMEDAD_GPIO);

        // Leer pH
        float ph = read_ph();

        // Controlar la bomba de agua
        if (humedad == 1) {
            gpio_set_level(BOMBA_AGUA_GPIO, 1);
            bomba_agua_encendida = true;
        } else {
            gpio_set_level(BOMBA_AGUA_GPIO, 0);
            bomba_agua_encendida = false;
        }

        // Controlar la bomba de solución ácida y básica
        if (ph < PH_MIN) {
            gpio_set_level(BOMBA_BASICA_GPIO, 1);
            bomba_basica_encendida = true;
        } else {
            gpio_set_level(BOMBA_BASICA_GPIO, 0);
            bomba_basica_encendida = false;
        }

        if (ph > PH_MAX) {
            gpio_set_level(BOMBA_ACIDA_GPIO, 1);
            bomba_acida_encendida = true;
        } else {
            gpio_set_level(BOMBA_ACIDA_GPIO, 0);
            bomba_acida_encendida = false;
        }

        // Informar el estado del sistema cada 5 segundos como lo indica el problema 
        static int counter = 0;
        if (counter == 0) {
            printf("pH: %.2f, %s\n", ph, (humedad == 0) ? "humedad correcta" : "humedad incorrecta");
            if (bomba_acida_encendida) {
                printf("Bomba de pHA encendida\n");
            }
            if (bomba_basica_encendida) {
                printf("Bomba de pHB encendida\n");
            }
            if (bomba_agua_encendida) {
                printf("Bomba de agua encendida\n");
            }
            counter = 5;
        }

        counter--;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/*==================[end of file]============================================*/
