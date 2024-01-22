/* Includes */
#include "main.h"
#include "usbd_cdc.h"
#include "SEGGER_RTT.h"
#include "func.h"
#include "bsp_delay.h"

uint8_t nintendoLogo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
                          0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
                          0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
                         };
int signal = 0; // signal from linux. default. do nothing. 1. read game info. 2. dump rom.
char gameTitle[17];
uint16_t cartridgeType = 0;
uint16_t romSize = 0;
uint16_t romBanks = 0;
uint16_t ramSize = 0;
uint16_t ramBanks = 0;
uint16_t ramEndAddress = 0;

void config_gpio_sig(void) {
	GPIO_Config_T GPIO_ConfigStruct;
	
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOC));
	GPIO_ConfigStruct.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct.pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_ConfigStruct.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOC, &GPIO_ConfigStruct);
}

void config_gpio_addr(void) {
	GPIO_Config_T GPIO_ConfigStruct_A;
	GPIO_Config_T GPIO_ConfigStruct_B;
	GPIO_Config_T GPIO_ConfigStruct_C;
	GPIO_Config_T GPIO_ConfigStruct_D;
	
	// GPIOC Group PC6/PC7/PC11/PC12
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOC));
	GPIO_ConfigStruct_C.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_C.pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_ConfigStruct_C.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOC, &GPIO_ConfigStruct_C);
	
	// GPIOB Group PB2/PB3/PB4/PB6/PB7
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOB));
	GPIO_ConfigStruct_B.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_B.pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | 
														GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_ConfigStruct_B.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct_B);
	
	// GPIOA Group PA8/PA9/PA10/PA13/PA14/PA15
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOA));
	GPIO_ConfigStruct_A.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_A.pin = GPIO_PIN_8	| GPIO_PIN_9	| GPIO_PIN_10 | 
														GPIO_PIN_13 | GPIO_PIN_14	| GPIO_PIN_15;
  GPIO_ConfigStruct_A.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOA, &GPIO_ConfigStruct_A);
	
	// GPIOD Group PD2
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOD));
	GPIO_ConfigStruct_D.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_D.pin = GPIO_PIN_2;
  GPIO_ConfigStruct_D.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOD, &GPIO_ConfigStruct_D);
}

void config_gpio_data(void) {
	GPIO_Config_T GPIO_ConfigStruct;
	
	// GPIOB PB8...15
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOB));
	GPIO_ConfigStruct.mode = GPIO_MODE_IN_FLOATING;
  GPIO_ConfigStruct.pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
													GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_ConfigStruct.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct);
}


void readHeader() {
	rd_mr_mreq_reset();
	
	for(uint16_t romAddress = 0x0134; romAddress <= 0x143; romAddress++) {
		char headerChar = (char)read_byte(romAddress);
		if ((headerChar >= 0x30 && headerChar <= 0x57) || // 0-9
			(headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			(headerChar >= 0x61 && headerChar <= 0x7A)) { // a-z
				gameTitle[(romAddress-0x0134)] = headerChar;
			}
	}
	gameTitle[16] = '\0';

	
	uint8_t logoCheck = 1;
	uint8_t x = 0;
	for (uint16_t romAddress = 0x0104; romAddress <= 0x133; romAddress++) {
		if (nintendoLogo[x] != read_byte(romAddress)) {
			logoCheck = 0;
			break;
		}
		x++;
	}
	cartridgeType = read_byte(0x0147);
	romSize = read_byte(0x0148);
	ramSize = read_byte(0x0149);
	
	// ROM banks
	romBanks = 2; //Default 32K
	if(romSize >= 1) {
		romBanks = 2 << romSize;
	}
	
	// RAM banks
	ramBanks = 0; // Default 0K RAM
	if (cartridgeType == 6) { ramBanks = 1; }
	if (ramSize == 2) { ramBanks = 1; }
  if (ramSize == 3) { ramBanks = 4; }
  if (ramSize == 4) { ramBanks = 16; }
  if (ramSize == 5) { ramBanks = 8; }
    
  // RAM end address
  if (cartridgeType == 6) { ramEndAddress = 0xA1FF; } // MBC2 512 bytes (nibbles)
  if (ramSize == 1) { ramEndAddress = 0xA7FF; } // 2K RAM
  if (ramSize > 1) { ramEndAddress = 0xBFFF; } // 8K RAM
	
	SEGGER_RTT_printf(0, "GameTitle: %c\n", gameTitle);
	SEGGER_RTT_printf(0, "cartridgeType: %d\n", cartridgeType);
	SEGGER_RTT_printf(0, "romSize: %d\n", romSize);
	SEGGER_RTT_printf(0, "ramSize: %d\n", ramSize);
	SEGGER_RTT_printf(0, "logoCheck: %d\n", logoCheck);
}
 
void loop() {
	// get control signal from usb
	switch(signal) {
		case 1:
			readHeader();
			break;
		default:
			break;
	}	
}
 
int main(void) {
	CDC_Init();
	config_gpio_sig();
	config_gpio_addr();
	config_gpio_data();
	while(1) {
		loop();
	}
}