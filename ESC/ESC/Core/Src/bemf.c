/*
 * bemf.c
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */


#include "bemf.h"

uint8_t BEMF_nulpunt(uint8_t stap, uint16_t *adc_buffer)
{
    uint16_t floating_voltage = 0;
    uint16_t nulpunt = (adc_buffer[0] + adc_buffer[1] + adc_buffer[2]) / 3;

    switch(stap)
    {
        case 1: // U naar V, W zweeft (BEMF daalt)
            floating_voltage = adc_buffer[2];
            return (floating_voltage < nulpunt);

        case 2: // U naar W, V zweeft (BEMF stijgt)
            floating_voltage = adc_buffer[1];
            return (floating_voltage > nulpunt);

        case 3:
            floating_voltage = adc_buffer[0];
            return (floating_voltage < nulpunt);

        case 4:
            floating_voltage = adc_buffer[2];
            return (floating_voltage > nulpunt);

        case 5:
            floating_voltage = adc_buffer[1];
            return (floating_voltage < nulpunt);

        case 6:
            floating_voltage = adc_buffer[0];
            return (floating_voltage > nulpunt);

        default: return 0;
    }
}

