
/**
 * @brief Convierte un dato de 32 bits a un arreglo de dígitos en formato BCD.
 *
 * @param[in] data El número de 32 bits a convertir.
 * @param[in] digits La cantidad de dígitos en los que se debe convertir el número.
 * @param[out] bcd_number Arreglo donde se almacenarán los dígitos BCD.
 *
 * @return 0 en caso de éxito, -1 si la cantidad de dígitos es inválida.
 * 
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 10/16/2024 | Document creation		                         |
 * 
 * @author Reyes Carlos (carlosarturoreyes69@gmail.com)
 */
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"

int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number);

/**
 * @brief Función principal de la aplicación.
 */
void app_main(void) {
    uint32_t data = 12345;  ///< Ejemplo de dato de 32 bits
    uint8_t bcd_digits[10]; ///< Arreglo para almacenar los dígitos BCD
    int8_t result = convertToBcdArray(data, 5, bcd_digits); ///< Convertir a BCD

    if (result == 0) {
        printf("Dígitos BCD: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", bcd_digits[i]);
        }
        printf("\n");
    } else {
        printf("Error: Cantidad de dígitos inválida.\n");
    }
}

/**
 * @brief Convierte un dato de 32 bits a un arreglo de dígitos en formato BCD.
 *
 * @param[in] data El número de 32 bits a convertir.
 * @param[in] digits La cantidad de dígitos en los que se debe convertir el número.
 * @param[out] bcd_number Arreglo donde se almacenarán los dígitos BCD.
 *
 * @return 0 en caso de éxito, -1 si la cantidad de dígitos es inválida.
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number) {
    if (digits == 0 || digits > 10) {
        // Verificar que la cantidad de dígitos sea válida (1-10)
        return -1; // Error: cantidad de dígitos inválida
    }

    // Se divide el número en dígitos BCD
    for (int i = 0; i < digits; i++) {
        // Extraer el dígito más significativo del número
        uint8_t digit = data % 10;

        // Almacenar el dígito BCD en el arreglo
        bcd_number[digits - 1 - i] = digit;

        // Dividir el número por 10 para obtener el siguiente dígito
        data /= 10;
    }

    return 0; // Éxito
}
