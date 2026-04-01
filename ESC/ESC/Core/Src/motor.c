/*
 * motor.c
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */
#include "motor.h"

void Motor_stap(uint8_t stap, uint16_t gas)
{
	//alle pwm waarde op 0 het ccr is capture compare register gemaakt voor pwm
	TIM1->CCR1 =0; //u
	TIM1->CCR2 =0; //v
	TIM1->CCR3 =0; //w

	//CCER is het capture compare enable register
	uint32_t ccer = 0;

	switch(stap)// 6 mogelijke stappen
	{
	case 1: //stroom u naar v fase
		TIM1->CCR1 = gas; //faseu
		ccer = TIM_CCER_CC1E | TIM_CCER_CC2NE; //cc1E u highside aan en V low side aan
		break;
	case 2: //stroom van u naar w
		TIM1->CCR1 = gas; //fase u
		ccer = TIM_CCER_CC1E | TIM_CCER_CC3NE; //cc1E u highside aan en w low side aan
		break;
	case 3: //stroom van v naar u
		TIM1->CCR2 = gas; //fase v
		ccer = TIM_CCER_CC2E | TIM_CCER_CC3NE;
	case 4:
		TIM1->CCR2 = gas; //fasev
		ccer = TIM_CCER_CC2E | TIM_CCER_CC1NE; //VHAAN ULAAN
	case 5:
		TIM1->CCR3 = gas; //fasew
		ccer = TIM_CCER_CC3E | TIM_CCER_CC1NE;
	case 6:
		TIM1->CCR3 = gas; //fasew
		ccer = TIM_CCER_CC3E | TIM_CCER_CC2NE;
	default:
		Motor_stop();
		return;
	}
	TIM1->CCER = ccer; //we schrijven juiste combo naar ccer register dat zal zorgen voor juiste configuratie
	TIM1->EGR = TIM1->EGR | TIM_EGR_COMG; //comg zet het excecuut register aan en zorgt dat het effectief wordt uitgevoerd
}

void Motor_stop(void)
{
	TIM1->CCER = 0; //alles uit
	TIM1->EGR = TIM1->EGR | TIM_EGR_COMG;
}
