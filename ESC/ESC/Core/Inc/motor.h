/*
 * motor.h
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"

void Motor_stap(uint8_t stap, uint16_t gas);
//de stap kan 1 tot en mot 6 zijn voor de verschillende configuraties
//de gas is van 0 tot en met 1000 dit hebben we bij dshot zo vastgelegd
void Motor_stop(void);

#endif /* INC_MOTOR_H_ */
