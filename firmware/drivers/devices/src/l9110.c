/**
 * @file l9110.c
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 * @brief 
 * @version 0.1
 * @date 2024-05-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

/*==================[inclusions]=============================================*/
#include "l9110.h"
#include "gpio_mcu.h"
#include "pwm_mcu.h"
/*==================[macros and definitions]=================================*/
#define MAX_F_SPEED 	100		/*!< Max foward speed  */
#define MAX_B_SPEED 	-100	/*!< Max backward speed */
#define PWM_FREQ 		50		/*!< PWM frequency (Hz) */
#define N_MOTORS		2		/*!< Number of motors */

#define A_1A			PWM_0
#define A_1B			PWM_1
#define B_1A			PWM_2
#define B_1B			PWM_3
/*==================[typedef]================================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions definition]==========================*/

/*==================[external data definition]===============================*/

/*==================[external functions definition]==========================*/
uint8_t L9110Init(gpio_t a_1a, gpio_t a_1b, gpio_t b_1a, gpio_t b_1b){
	PWMInit(A_1A, a_1a, PWM_FREQ);
	PWMInit(A_1B, a_1b, PWM_FREQ);
	PWMInit(B_1A, b_1a, PWM_FREQ);
	PWMInit(B_1B, b_1b, PWM_FREQ);

	return 1;
}

uint8_t L9110SetSpeed(l9110_motor_t motor, int8_t speed){
	uint8_t err = 0;

	switch(motor){
	case MOTOR_A:
		if(speed == 0){
			PWMSetDutyCycle(A_1A, speed);
			PWMSetDutyCycle(A_1B, speed);
		}
		if(speed > 0){
			if (speed > MAX_F_SPEED) speed = MAX_F_SPEED;
			PWMSetDutyCycle(A_1A, speed);
			PWMSetDutyCycle(A_1B, 0);
		}
		if(speed < 0){
			if (speed < MAX_B_SPEED) speed = MAX_B_SPEED;
			PWMSetDutyCycle(A_1A, 0);
			PWMSetDutyCycle(A_1B, -speed);
		}
		break;
	case MOTOR_B:
		if(speed == 0){
			PWMSetDutyCycle(B_1A, speed);
			PWMSetDutyCycle(B_1B, speed);
		}
		if(speed > 0){
			if (speed > MAX_F_SPEED) speed = MAX_F_SPEED;
			PWMSetDutyCycle(B_1A, speed);
			PWMSetDutyCycle(B_1B, 0);
		}
		if(speed < 0){
			if (speed < MAX_B_SPEED) speed = MAX_B_SPEED;
			PWMSetDutyCycle(B_1A, 0);
			PWMSetDutyCycle(B_1B, -speed);
		}
		break;
	default:
		err = 1;
		break;
	}

	return err;
}

uint8_t L293DeInit(void){
	PWMOff(A_1A);
	PWMOff(A_1B);
	PWMOff(B_1A);
	PWMOff(B_1B);
	return 1;
}
