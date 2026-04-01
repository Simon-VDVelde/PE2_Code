/*
 * bemf.h
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */

#ifndef INC_BEMF_H_
#define INC_BEMF_H_

#include "main.h"
//zo gekozen dat dit de helft is van de 12 bit adc omdat we aftakken op half de voedingspanning
#define BEMF_treshold 2048


uint8_t BEMF_nulpunt(uint8_t stap, uint16_t *adc_buffer);
//functie geeft 1  als nulpunt van fase bereikt is

#endif /* INC_BEMF_H_ */
