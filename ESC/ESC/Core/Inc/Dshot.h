/*
 * Dshot.h
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */

#ifndef SRC_DSHOT_H_
#define SRC_DSHOT_H_

#include "main.h"

#define DSHOT_FRAME_SIZE 16
#define DSHOT_THRESHOLD 45  // Bij 48MHz clock


typedef struct {
    uint16_t gas;
    uint8_t  telemetry;
    uint8_t  geldig;
    uint8_t  crc;
} DShot_Data_t;

// Functies
void DShot_Init(TIM_HandleTypeDef *htim);        //tim 1 juist zetten op pa0
DShot_Data_t DShot_Decode(uint32_t *dma_buffer); //om het frame om te zetten in gas

#endif /* INC_DSHOT_H_ */

