#ifndef L9110_H_
#define L9110_H_
/** \addtogroup Drivers_Programable Drivers Programable
 ** @{ */
/** \addtogroup Drivers_Devices Drivers devices
 ** @{ */
/** \addtogroup L9110 L9110
 ** @{ 
 * @brief  Double H bridge driver
 *
 * This driver provide functions to configure and control a dual DC motor driver
 * using the L9110.
 *
 * @author Albano Peñalva
 *
 * @note Hardware connections:
 *
 * |   	L9110		|   EDU-CIAA	|
 * |:--------------:|:--------------|
 * | 	VCC		 	|	5V			|
 * | 	GND		 	| 	GND			|
 * | 	B-1A	 	| 	GPIO_X		|
 * | 	B-1B	 	| 	GPIO_X		|
 * | 	A-1A	 	| 	GPIO_X		|
 * | 	A-1B	 	| 	GPIO_X		|
 *
 * @section changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 17/05/2024 | Document creation		                         |
 *
 */

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "gpio_mcu.h"

/*==================[macros]=================================================*/

/*==================[typedef]================================================*/
/**
 * @brief  Motors
 */
typedef enum
{
	MOTOR_A, 	/*!< Motor A */
	MOTOR_B,  	/*!< Motor B */
} l9110_motor_t;

/*==================[external data declaration]==============================*/

/*==================[external functions declaration]=========================*/
/**
 * @brief  		Initializes L9110 driver
 * @param[in]  	None
 * @retval 		1 when success, 0 when fails
 */

/**
 * @brief  		Initializes L9110 driver 
 * 
 * @param a_1a	gpio connected to A-1A pin 
 * @param a_1b	gpio connected to A-1B pin 
 * @param b_1a	gpio connected to B-1A pin 
 * @param b_1b	gpio connected to B-1B pin 
 * @return 		1 when success, 0 when fails 
 */
uint8_t L9110Init(gpio_t a_1a, gpio_t a_1b, gpio_t b_1a, gpio_t b_1b);

/**
 * @brief  		Initializes L9110 driver
 * @param[in]  	motor: 	motor to be configured
 * @param[in]  	speed: 	from -100 to 100
 * 						0: 			stop
 * 						1 to 100: 	foward
 * 						-1 to -100: backward
 * @retval 		1 when success, 0 when fails
 */
uint8_t L9110SetSpeed(l9110_motor_t motor, int8_t speed);

/**
 * @brief  	De-initializes L9110 Driver
 * @param	None
 * @retval 	1 when success, 0 when fails
 */
uint8_t L9110DeInit(void);

/** @} doxygen end group definition */
/** @} doxygen end group definition */
/** @} doxygen end group definition */
#endif /* L9110_H_ */

/*==================[end of file]============================================*/
