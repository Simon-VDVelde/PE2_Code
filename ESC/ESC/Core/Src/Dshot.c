/*
 * Dshot.C
 *
 *  Created on: Mar 12, 2026
 *      Author: tijme
 */

#include "Dshot.h"


extern uint32_t dshot_dma_buffer[DSHOT_FRAME_SIZE];

uint8_t dshot_calculate_crc(uint16_t frame)           //  Met deze funcite berekenen we de CRC dezelfde berekening doen we op de flight contorler en kunnen we later controleren met wat binnenkomt
{
	uint16_t data = frame >> 4; // 12 bits met informatie de crc bit eraf geshift
	uint8_t crc = (data ^ (data >> 4) ^ (data >> 8)) & 0x0F;  //enkel de laatste 4 bits masken
	return crc;
}

//TIM1 starten
void DShot_Init(TIM_HandleTypeDef *htim)  //pointer naar data+Llengte
{
	HAL_TIM_IC_Start_DMA(htim, TIM_CHANNEL_1, dshot_dma_buffer, DSHOT_FRAME_SIZE);
}


DShot_Data_t DShot_Decode(uint32_t *dma_buffer) //met behulp van struct in header
{
	DShot_Data_t result = {0};  //struct uit header file aanmaken
    uint16_t dshot_frame = 0;   //frame leegmaken

	  //de hoog en laag tijden vertalen naar 1 en 0 met de ingestelde treshhold
	  for (int i = 0; i < DSHOT_FRAME_SIZE; i++)  //dus 16 keer = frame size
	  {
		  if (dma_buffer[i] > DSHOT_THRESHOLD) //als het een logische 1 is in dma
		  {
			  dshot_frame = dshot_frame | (1 << (15 - i)); // we vulle de 1 in bij het frame
		  }
	  }

	  //opdelen in gas telemetry en crc
	  uint8_t received_crc = dshot_frame & 0x0F; //gekregen CRC uit het frame halen4
	  uint16_t gas_extra = dshot_frame >> 5;      // crc en telemtry  bits eraf 4 voor crc 1 telemetry de extra staat voor eventuele commmandos zoals piepjes of lichtjes maar dat hebben wij niet dit is voor zien voor waarde 0- 48


	  if (dshot_calculate_crc(dshot_frame) == received_crc)
	  {
		  result.geldig = 1;
	  	  result.telemetry = (dshot_frame >> 4) & 0x01; //maar de ene enkele bit nodig
	  	  result.crc = received_crc;
	  	 if ( gas_extra >= 48)
	  			  {
	  				  result.gas = (gas_extra - 48) * 1000 / 1999; //de array van TIM1 staat op 1000 dus het moet lopen van 0 tot 1000 en niet van 48-2047 dus 2047-48=1999 is de max waarde}
	  			  }
	  	 else
	  	 {
	  		 	 	 result.gas = 0; //nu gebruiken we bit 0-48 dus echt enkel om een stop door te sturen kunnen later eventueel nog andere commando's mee gemaakt worden
	  	 }
	  }
	  else
	  {
	          result.geldig = 0;
	  }
	  return result;
}


