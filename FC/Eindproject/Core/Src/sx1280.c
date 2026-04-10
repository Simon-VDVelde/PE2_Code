#include "sx1280.h"
#include "main.h"



extern SPI_HandleTypeDef hspi2;
ELRS_State_t elrs_state = ELRS_STATE_DISCONNECTED;
uint8_t current_hop_index = 0;
uint8_t payload_buffer[8];
uint32_t last_packet_time = 0;
uint16_t rc_roll = 992;     // 992 is het wiskundige midden in 11-bit(waarde van 0-2047)
uint16_t rc_pitch = 992;
uint16_t rc_yaw = 992;
uint16_t rc_throttle = 172; // 172 is vaak de absolute minimumwaarde

static void SX1280_SetRfFrequency(uint32_t frequency);


/*
 * @brief Initialiseert de SX1280 met de basis ELRS instellingen
 */
void SX1280_Init(void) {
    // 1. Harde Reset (functie had je al)
	SX1280_nRST();

    // 2. Chip in Standby Mode zetten (RC klok) - VERPLICHT voor we settings aanpassen
    uint8_t standby_cmd[2] = {SX1280_SET_STANDBY, 0x00};
    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, standby_cmd, 2, HAL_MAX_DELAY);
    SX1280_Deselect();

    // 3. Pakket Type: 0x01 = LoRa Mode
    SX1280_SetPacketType(0x01);

    // 4. Modulatie: SF5 (0x50), Bandbreedte 800kHz (0x18), CR 4/6 (0x02)
    SX1280_SetModulationParams(0x50, 0x18, 0x02);

    // 5. Pakket parameters: Preamble 12, Implicit Header (0x01), 8 bytes payload, CRC Aan (0x20), Invert IQ (0x40)
    SX1280_SetPacketParams(0x0C, 0x01, 0x08, 0x20, 0x40);

    // 6. Frequentie: Test frequentie instellen (bijv. 2.44 GHz)
    SX1280_SetRfFrequency(2440000000);

    // 7. Interrupts instellen!
    // 0x4002 = Globaal luisteren naar SyncWordValid én RxDone (Pakketje binnen)
    // 0x0002 = Koppel de RxDone trigger fysiek aan de DIO1 pin
    SX1280_SetDioIrqParams(0x4002, 0x0002, 0x0000, 0x0000);
}

void SX1280_BUSY(void)
{
    while(HAL_GPIO_ReadPin(SX1280_Bussy_GPIO_Port, SX1280_Bussy_Pin) == 1);
}



void SX1280_Select(void)
{
    HAL_GPIO_WritePin(SX1280_CS_GPIO_Port, SX1280_CS_Pin, 0);
}

void SX1280_Deselect(void)
{
    HAL_GPIO_WritePin(SX1280_CS_GPIO_Port, SX1280_CS_Pin, GPIO_PIN_SET);
}

void SX1280_nRST(void)
{
    HAL_GPIO_WritePin(SX1280_nRST_GPIO_Port, SX1280_nRST_Pin, 0);
    HAL_Delay(20);
    HAL_GPIO_WritePin(SX1280_nRST_GPIO_Port, SX1280_nRST_Pin, 1);
    SX1280_BUSY();
}


void SX1280_WriteRegister(uint16_t address, uint8_t value)
{
    uint8_t buf[4];
    buf[0] = SX1280_WRITEREGISTER;             	 //header file
    buf[1] = (uint8_t)(address >> 8);  		// MSb
    buf[2] = (uint8_t)(address & 0xFF);		// LSB
    buf[3] = value;                    			//data

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, buf, 4, HAL_MAX_DELAY);  ///versture buffer
    SX1280_Deselect();
}


uint8_t SX1280_ReadRegister(uint16_t address)  //instellen sx1280
{
    uint8_t status;
    uint8_t addr_buf[3];
    uint8_t NOP = 0x00;              // NOP byte
    uint8_t result = 0;

    addr_buf[0] = SX1280_READREGISTER;
    addr_buf[1] = (uint8_t)(address >> 8);
    addr_buf[2] = (uint8_t)(address & 0xFF);

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, addr_buf, 3, HAL_MAX_DELAY);

    //nop zegt datasheet om te doen
    HAL_SPI_Transmit(&hspi2, &NOP, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &result, 1, HAL_MAX_DELAY);  //geeft register
    SX1280_Deselect();
    return result;
}

void SX1280_ReadBuffer(uint8_t offset, uint8_t *data, uint8_t size)
{
	uint8_t NOP = 0x00;
	uint8_t READBUFFER = SX1280_READBUFFER;
	SX1280_BUSY();
    SX1280_Select();

    HAL_SPI_Transmit(&hspi2, &READBUFFER, 1, HAL_MAX_DELAY);//1byte sturen
    HAL_SPI_Transmit(&hspi2, &offset, 1, HAL_MAX_DELAY);//zelf offset kiezen tussen 0 en 256
    HAL_SPI_Transmit(&hspi2, &NOP, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, data, size, HAL_MAX_DELAY);
    SX1280_Deselect();
}


//frequentie Hz 2400000000 2.4GHz

static void SX1280_SetRfFrequency(uint32_t frequency)
{
    // We gebruiken 52MHZ kristal: (freq << 18) / 52000000
    uint32_t rfwaarde = (uint32_t)(((uint64_t)frequency << 18) / 52000000);

    uint8_t buf[4];
    buf[0] = SX1280_SETRFFREQUENCY;
    buf[1] = (uint8_t)( rfwaarde >> 16); 	// MSB
    buf[2] = (uint8_t)( rfwaarde >> 8);  	// Middelste byte
    buf[3] = (uint8_t)( rfwaarde & 0xFF);   // LSB

    SX1280_BUSY();
    SX1280_Select();

    HAL_SPI_Transmit(&hspi2, buf, 4, HAL_MAX_DELAY);

    SX1280_Deselect();
}

/*
 * @brief Zet de chip in LoRa of FLRC modus
 */
void SX1280_SetPacketType(uint8_t packetType) {
    uint8_t buf[2];
    buf[0] = SX1280_SETPACKET_TYPE;
    buf[1] = packetType;

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, buf, 2, HAL_MAX_DELAY);
    SX1280_Deselect();
}

/*
 * @brief Stel Spreading Factor, Bandwidth en Coding Rate in
 */
void SX1280_SetModulationParams(uint8_t sf, uint8_t bw, uint8_t cr) {
    uint8_t buf[4];
    buf[0] = SX1280_SETMODULATIONPARAMS;
    buf[1] = sf;
    buf[2] = bw;
    buf[3] = cr;

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, buf, 4, HAL_MAX_DELAY);
    SX1280_Deselect();
}

/*
 * @brief Stel Preamble, Header type, Lengte en CRC in
 */
void SX1280_SetPacketParams(uint8_t preambleLen, uint8_t headerType, uint8_t payloadLen, uint8_t crc, uint8_t invertIQ) {
    uint8_t buf[8];
    buf[0] = SX1280_SET_PACKET_PARAMS;
    buf[1] = preambleLen;
    buf[2] = headerType;
    buf[3] = payloadLen;
    buf[4] = crc;
    buf[5] = invertIQ;
    buf[6] = 0x00; // ongebruikt in LoRa, vereist door datasheet
    buf[7] = 0x00; // ongebruikt in LoRa, vereist door datasheet

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, buf, 8, HAL_MAX_DELAY);
    SX1280_Deselect();
}

/*
 * @brief Koppel specifieke gebeurtenissen (zoals RxDone) aan de fysieke DIO pinnen
 */
void SX1280_SetDioIrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask) {
    uint8_t buf[9];
    buf[0] = SX1280_SET_DIO_IRQ_PARAMS;
    buf[1] = (uint8_t)(irqMask >> 8);
    buf[2] = (uint8_t)(irqMask & 0xFF);
    buf[3] = (uint8_t)(dio1Mask >> 8);
    buf[4] = (uint8_t)(dio1Mask & 0xFF);
    buf[5] = (uint8_t)(dio2Mask >> 8);
    buf[6] = (uint8_t)(dio2Mask & 0xFF);
    buf[7] = (uint8_t)(dio3Mask >> 8);
    buf[8] = (uint8_t)(dio3Mask & 0xFF);

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, buf, 9, HAL_MAX_DELAY);
    SX1280_Deselect();
}

/*
 * @brief Deze functie wordt razendsnel aangeroepen door de STM32 hardware interrupt
 * zodra de SX1280 een pakketje foutloos heeft ontvangen.
 */
void ELRS_HandleRxInterrupt(void)
{
    // 1. Wis de interrupt vlaggen op de SX1280
    // Als je dit niet doet, blijft de SX1280 denken dat hij vol zit en crasht de loop.
    // 0x03FF = Clear All IRQs (IRQ register is 2 bytes op de sx1280)
    uint8_t clear_irq_buf[3] = {SX1280_CLRIRQSTATUS, 0x03, 0xFF};

    SX1280_Bussy();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, clear_irq_buf, 3, HAL_MAX_DELAY);
    SX1280_Deselect();

    // 2. Haal het pakketje (8 bytes) op uit de SX1280 buffer
    // Bij LoRa Mode staat het binnengekomen pakketje altijd netjes klaar vanaf offset 0x00
    SX1280_ReadBuffer(0x00, payload_buffer, 8);
    ELRS_ParsePayload();

    // 3. Update de connectie status
    elrs_state = ELRS_STATE_CONNECTED;
    last_packet_time = HAL_GetTick(); // Sla de huidige processortijd op (belangrijk voor failsafe!)

    // [HIER KOMT LATER DE CODE OM DE DATA OM TE ZETTEN NAAR JOYSTICK BEREKENINGEN]

    // 4. Zet de SX1280 direct weer op luisteren voor het volgende pakketje
    // In een echt ELRS protocol hoppen we hier eerst naar de volgende frequentie,
    // maar voor nu zetten we hem gewoon weer 'Aan'.
    SX1280_SetRx();
}

void SX1280_SetRx(void)
{
    uint8_t buf[4];
    buf[0] = SX1280_SETRX;
    buf[1] = 0x01; // 1ms
    buf[2] = 0xFF;
    buf[3] = 0xFF; // continu blijven sampelen

    SX1280_BUSY();
    SX1280_Select();
    HAL_SPI_Transmit(&hspi2, buf, 4, HAL_MAX_DELAY);
    SX1280_Deselect();
}

/*
 * @brief Zet de 8-byte payload om in 11-bit joystick kanalen
 */
void ELRS_ParsePayload(void) {
    // We slaan byte 0 over, dat is de 'Header' van het pakketje.
    // De data zit in payload_buffer[1] t/m [6]

    rc_roll     = (payload_buffer[1]       | payload_buffer[2] << 8) & 0x07FF;
    rc_pitch    = (payload_buffer[2] >> 3  | payload_buffer[3] << 5) & 0x07FF;
    rc_throttle = (payload_buffer[3] >> 6  | payload_buffer[4] << 2 | payload_buffer[5] << 10) & 0x07FF;
    rc_yaw      = (payload_buffer[5] >> 1  | payload_buffer[6] << 7) & 0x07FF;

    // --- Optioneel: Voorbereiding voor de PID-loop ---
    // rc_throttle is nu een getal tussen ~172 en ~1811.
    // Straks moeten we deze mappen naar DSHOT (48 tot 2047).
}
