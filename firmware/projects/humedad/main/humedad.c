#include <stdio.h>
#include <esp_system.h>
#include <driver/adc.h>
#include <freertos/FreeRTOS.h> 
#include <freertos/task.h>

#define SOIL_HUMIDITY_PIN 16

void app_main()
{
    // Initialize ADC
    adc1_config_channel_atten(SOIL_HUMIDITY_PIN, ADC_ATTEN_DB_11);
    adc1_config_width(ADC_WIDTH_BIT_12);

    while (1) {
        // Read soil humidity value
        int soilHumidityValue = adc1_get_raw(SOIL_HUMIDITY_PIN);

        // Convert value to percentage
        float soilHumidityPercentage = (soilHumidityValue / 4095.0) * 100.0;

        // Print result to console
        printf("Soil Humidity: %.2f%%\n", soilHumidityPercentage);

        // Wait 1 second before taking the next reading
        vTaskDelay(1000 / portTICK_PERIOD_MS); // This should work now
    }
}