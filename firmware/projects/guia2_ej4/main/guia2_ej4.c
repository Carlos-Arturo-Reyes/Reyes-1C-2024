#include "esp_system.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/uart.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"

#define CHANNEL ADC_CHANNEL_0
#define SAMPLE_FREQUENCY 500
#define BAUD_RATE 115200

static QueueHandle_t uxQueue;

void IRAM_ATTR adc_isr_handler(void* arg)
{
    uint32_t adc_reading = 0;
    adc_channel_t adc_channel = CHANNEL;
    adc1_get_raw((adc_channel_t)adc_channel, (adc_attenuation_t)ADC_ATTEN_DB_11, &adc_reading);
    xQueueSendFromISR(uxQueue, &adc_reading, NULL);
}

void app_main()
{
    // Initialize the UART
    uart_config_t uart_config = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM_1, &uart_config);
    uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM_1, 256, 0, 0, NULL, 0);

    // Initialize the ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(CHANNEL, ADC_ATTEN_DB_11);

    // Initialize the queue
    uxQueue = xQueueCreate(10, sizeof(uint32_t));

    // Set up the timer
    const esp_timer_create_args_t timer_args = {
        .callback = &adc_isr_handler,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "adc_timer"
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, 1000000 / SAMPLE_FREQUENCY));

    // Start the continuous ADC conversion
    analog_input_config_t config = {
        .input = CHANNEL,
        .mode = ADC_CONTINUOUS,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = SAMPLE_FREQUENCY
    };
    AnalogInputInit(&config);
    AnalogStartContinuous(CHANNEL);

    // Main loop
    while (1) {
        uint32_t adc_reading;
        xQueueReceive(uxQueue, &adc_reading, portMAX_DELAY);
        uint16_t voltage = AnalogRaw2mV(adc_reading);
        char data[10];
        sprintf(data, "%d\n", voltage);
        uart_write_bytes(UART_NUM_1, data, strlen(data));
    }
}