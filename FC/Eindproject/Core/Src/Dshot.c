/*
* dshotsturen.c
*
* Created on: 27 mrt 2026
* Author: tijme
*/

/*
* Deze functie gaat dshot uitsturen
* Het zal naar al de

* Specs
* DShot300 sturen (als dit werkt kan dit altijd worden aangepast
* Frame: 16Bit
* 0-10 throthle bits
* - 0 tot 46 speciale instructies
* - 47 tot 2047 puur throthle dus 47 is af 2000 is vol gas
* 11 telemetry bits
* - kan gebruikt worden om via fc data terug te vragen bij ons is dit momenteel de bedoeling de overcurrent terug te vragen
* 12-15 crc check
*
*/

// m0=M1(PA0), m1=M2(PA2), m2=M3(PD13), m3=M4(PD12)

#include "Dshot.h"

/*
* als de timers niet appart staan ingesteld op 32 en 16
* uint32_t motor1_2_buffer[17]; // TIM2 32bit dus 16 dshot en 1 voor pauze
* uint16_t motor3_4_buffer[17]; // TIM4 16 bit
*/

//beide ingesteld op 16 en dus het eerste is motor id
uint32_t motor1_2_buffer[2][17];
uint16_t motor3_4_buffer[2][17];

/* we hebben dshot300 en de timer van ioc was op 50Hz gezet d */
#define DS_0 125 // 37.5% van 166
#define DS_1 250 // 75% van 166
#define DSHOT_BUF_SIZE 17

/* buffer vullen */
void update_motor_buffer(uint8_t motor_id, uint16_t throttle, uint8_t telemetry)
{
	uint16_t telemetry_bit;
	uint16_t gastelemetry;
	uint16_t dshotpakket;
	//0-48 geeen gas
	if (throttle > 0 && throttle < 48)
	{
		throttle = 48;
	}
	//2047 is volle gas
	if (throttle > 2047)
	{
		throttle = 2047;
	}

	//telemtry
	if (telemetry ==1){
		telemetry_bit = 1;
	} else {
		telemetry_bit = 0;
	}

	// gas + telemetru
	gastelemetry = (throttle << 1) | telemetry_bit;

	//uitleg crc berekening zie obsidian
	uint16_t crc = (gastelemetry ^ (gastelemetry >> 4) ^ (gastelemetry >> 8)) & 0x0F;

	//alles samen
	dshotpakket = (gastelemetry << 4 ) | crc;

	//buffer vullen
	for (int i = 0; i < 16; i++)
	{
		//0x8000 = 1000 0000 0000 0000 dus 16 bit waarbij ik de masker altijd naar juiste schuif
		uint16_t masker = (0x8000 >> i);
		uint32_t pwm_value = ((dshotpakket & masker) != 0) ? DS_1 : DS_0;

		// Vul de juiste buffer afhankelijk van de motor_id (0 en 1 zijn TIM2, 2 en 3 zijn TIM4)
		if (motor_id == 0 || motor_id == 1) {
			motor1_2_buffer[motor_id][i] = pwm_value;
		} else if (motor_id == 2 || motor_id == 3) {
			motor3_4_buffer[motor_id - 2][i] = (uint16_t)pwm_value; // Let op de -2 indexering!
		}

	}

	// de laatste bit moet pauze zijn
	if (motor_id == 0 || motor_id == 1) {
		motor1_2_buffer[motor_id][16] = 0;
	} else if (motor_id == 2 || motor_id == 3) {
		motor3_4_buffer[motor_id - 2][16] = 0;
	}


}

/* 4. Het verzenden */
void send_dshot() {
	// DMA kan niet aan dcache van de chip vandaar alles van de dcache naar de ram zetten voor de nieuwe data
	SCB_CleanDCache_by_Addr((uint32_t*)motor1_2_buffer, sizeof(motor1_2_buffer));
	SCB_CleanDCache_by_Addr((uint32_t*)motor3_4_buffer, sizeof(motor3_4_buffer));
	// DMA transfers starten
	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_1, (uint32_t*)motor1_2_buffer[0], 17);
	HAL_TIM_PWM_Start_DMA(&htim2, TIM_CHANNEL_3, (uint32_t*)motor1_2_buffer[1], 17);
	HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_1, (uint32_t*)motor3_4_buffer[0], 17);
	HAL_TIM_PWM_Start_DMA(&htim4, TIM_CHANNEL_2, (uint32_t*)motor3_4_buffer[1], 17);
}





