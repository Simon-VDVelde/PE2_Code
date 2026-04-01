/*
 * bemf.c
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */


#include "bemf.h"


uint8_t BEMF_nulpunt(uint8_stap, uint16_t *adc_buffer)
{
	//adc_buffer[0] = U-fase
	//adc_buffer[1] = v-fase
	//adc_buffer[2] = W-fase
	uint16_t volt = 0;
	uint16_t adc_buffer_tempu;
	uint16_t adc_buffer_tempv;
	switch(stap)
	{
	        case 1: // U=High, V=Low W fase zweeft
	            voltage = adc_buffer[0];



	    return 0;
	}

