/*
 * BMI330.c
 * Created on: 28 feb 2026
 * Author: simon
 */

#include "BMI330.h"

extern SPI_HandleTypeDef hspi1;

void BMI330_Init(void){
    uint16_t id;
    uint16_t feature_status = 0;
    uint32_t timeout = 100;

    // Start de feature engine op
    BMI330_WriteReg(BMI330_FEATURE_IO2, 0x012C);
    BMI330_WriteReg(BMI330_FEATURE_IO_STATUS, 0x01);
    BMI330_WriteReg(BMI330_FEATURE_CTRL, 0x01);

    do {
        BMI330_ReadReg(BMI330_FEATURE_IO1, &feature_status, 1);
        HAL_Delay(1);
        timeout--;
    } while ((feature_status != 0x0001) && (timeout > 0));

    if (timeout == 0){
        printf("Fout: Feature engine start niet op\n");
    }

    BMI330_ReadReg(BMI330_CHIP_ID, &id, 1);
    if(id != BMI330_ID) {
        printf("Fout: Verkeerd Chip ID\n");
    }

    // Stel de accelerometer in (8G range) en gyroscoop (2000 dps)
    BMI330_WriteReg(BMI330_ACC_CONF, 0x70AB);
    BMI330_WriteReg(BMI330_GYR_CONF, 0x70CB);

    // --- Interrupt Instellingen ---
	// 1. Configureer de INT1 pin (Push-Pull, Active High)
	BMI330_WriteReg(BMI330_IO_INT_CTRL, 0x05);

	// 2. Zet latched interrupts uit (we willen gewoon een korte puls)
	BMI330_WriteReg(BMI330_INT_CONF, 0x00);

	// 3. Map de "Data Ready" interrupt naar de INT1 pin
	// (Check in je BMI330 datasheet of 0x80 inderdaad de DRDY bit is voor INT_MAP1.
	// Bij de meeste Bosch sensoren is bit 7 de data ready flag voor INT1).
	BMI330_WriteReg(BMI330_INT_MAP1, 0x80);
}

void BMI330_WriteReg(uint8_t reg, uint16_t val){
    uint8_t data[3] = {reg & 0x7F, val & 0xFF, (val >> 8) & 0xFF};
    HAL_GPIO_WritePin(BMI_CS_GPIO_Port, BMI_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, data, 3, 100);
    HAL_GPIO_WritePin(BMI_CS_GPIO_Port, BMI_CS_Pin, GPIO_PIN_SET);
}

void BMI330_ReadReg(uint8_t reg, uint16_t *pData, uint8_t len) {
    uint8_t address = reg | 0x80;
    uint8_t dummy;
    uint8_t buffer[len * 2];

    HAL_GPIO_WritePin(BMI_CS_GPIO_Port, BMI_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &address, 1, 100);
    HAL_SPI_Receive(&hspi1, &dummy, 1, 100);
    HAL_SPI_Receive(&hspi1, buffer, len * 2, 100);
    HAL_GPIO_WritePin(BMI_CS_GPIO_Port, BMI_CS_Pin, GPIO_PIN_SET);

    for (int i = 0; i < len; i++) {
        pData[i] = (uint16_t)((buffer[2 * i + 1] << 8) | buffer[2 * i]);
    }
}

float BMI330_GetTemp(void) {
    uint16_t temperature;
    BMI330_ReadReg(BMI330_TEMP_DATA, &temperature, 1);
    return (float)temperature / 256.0f + 23.0f;
}

/*
 * @brief 1 Algemene functie om alles razendsnel uit te lezen in de hoofdlus
 * @param frame De struct waar de ruwe g/dps data in moet
 * @param roll Pointer naar de berekende roll hoek
 * @param pitch Pointer naar de berekende pitch hoek
 */
void BMI330_Update(BMI330_Frame *frame, float *roll, float *pitch) {
    // We starten bij het X-register van de accelerometer (0x03)
    // We lezen 12 bytes: 6 voor Accel (X,Y,Z) en direct daaropvolgend 6 voor Gyro (X,Y,Z)
    uint8_t address = BMI330_ACC_DATA_X | 0x80;
    uint8_t dummy;
    uint8_t raw_data[12];

    // Supersnelle burst-read over SPI
    HAL_GPIO_WritePin(BMI_CS_GPIO_Port, BMI_CS_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi1, &address, 1, 100);
    HAL_SPI_Receive(&hspi1, &dummy, 1, 100);
    HAL_SPI_Receive(&hspi1, raw_data, 12, 100);
    HAL_GPIO_WritePin(BMI_CS_GPIO_Port, BMI_CS_Pin, GPIO_PIN_SET);

    // 1. Bit-shifting en omrekenen naar juiste eenheden (4096 = 8g, 16.384 = 2000dps)
    frame->acc_x = (float)((int16_t)((raw_data[1] << 8) | raw_data[0])) / 4096.0f;
    frame->acc_y = (float)((int16_t)((raw_data[3] << 8) | raw_data[2])) / 4096.0f;
    frame->acc_z = (float)((int16_t)((raw_data[5] << 8) | raw_data[4])) / 4096.0f;

    frame->gyr_x = (float)((int16_t)((raw_data[7] << 8) | raw_data[6])) / 16.384f;
    frame->gyr_y = (float)((int16_t)((raw_data[9] << 8) | raw_data[8])) / 16.384f;
    frame->gyr_z = (float)((int16_t)((raw_data[11] << 8) | raw_data[10])) / 16.384f;

    // 2. Direct de hoeken berekenen voor de 'Outer Loop' (Angle mode)
    // Vermenigvuldig met 180 / PI om van radialen naar graden te gaan
    *roll = atan2f(frame->acc_y, frame->acc_z) * (180.0f / 3.1415926535f);

    float acc_z_y_combined = sqrtf(frame->acc_y * frame->acc_y + frame->acc_z * frame->acc_z);
    *pitch = atan2f(-frame->acc_x, acc_z_y_combined) * (180.0f / 3.1415926535f);
}
