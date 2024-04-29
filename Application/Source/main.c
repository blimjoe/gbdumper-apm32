/* Includes */
#include "main.h"
#include "usbd_cdc.h"
#include "SEGGER_RTT.h"
#include "func.h"
#include "bsp_delay.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

uint8_t nintendoLogo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
                          0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
                          0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
                         };

char gameTitle[16];
uint16_t cartridgeType = 0;
uint16_t romSize = 0;
uint16_t romBanks = 0;
uint16_t ramSize = 0;
uint16_t ramBanks = 0;
uint16_t ramEndAddress = 0;
uint8_t sdBuffer[512];
unsigned long cartSize_gba;
uint8_t saveType;
char cartID[5]; 


void Delay(void){
    volatile uint32_t delay = 0xfffff;
    while(delay--);
}

// GB/GBC
void readHeader() {
	rd_wr_mreq_reset();
	
	for(uint16_t romAddress = 0x0134; romAddress <= 0x143; romAddress++) {
		char headerChar = (char)read_byte(romAddress);
		if (headerChar >= 0x0020 && headerChar <= 0x7E) {
			gameTitle[(romAddress-0x0134)] = headerChar;
		}
	}
	gameTitle[15] = '\0';

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

	USBD_TxData(USBD_EP_1, (uint8_t*)gameTitle, strlen(gameTitle));
	memset(gameTitle, 0, sizeof(gameTitle));
}

void dumpRom(void) {
	rd_wr_mreq_reset();
	uint16_t romAddress = 0;
	for(uint16_t bank = 1; bank < romBanks; bank++) {
		if(cartridgeType >= 5) { // MBC2 and above
			write_byte(0x2100, bank); //set ROM bank
		} else {
			write_byte(0x6000, 0); //set ROM mode
			write_byte(0x4000, bank>>5); //set bits 5 & 6 (01100000) of ROM bank
			write_byte(0x2000, bank & 0x1F); // Set bits 0 & 4 (00011111) of ROM bank
		} if(bank > 1) {
			romAddress = 0x4000;
		}
		// Read up to 7FFF per bank
		while (romAddress <= 0x7FFF) {
			uint8_t readData[64];
			for(int i = 0; i < 64; i++){
				readData[i] = read_byte(romAddress+i);
			}
			// usb write
			USBD_TxData(USBD_EP_1, readData, sizeof(readData));
			romAddress += 64;
		}	
	}
}

void readram(void) {
	rd_wr_mreq_reset();
	
	read_byte(0x0134);
    
  // if cartridge have RAM test
  if (ramEndAddress > 0) {
		if (cartridgeType <= 4) { // MBC1
			write_byte(0x6000, 1); // Set RAM Mode
    }
      
    // Initialise MBC
    write_byte(0x0000, 0x0A);
      
    // Switch RAM banks
    for (uint8_t bank = 0; bank < ramBanks; bank++) {
      write_byte(0x4000, bank);
        
      // Read RAM
      for (uint16_t ramAddress = 0xA000; ramAddress <= ramEndAddress; ramAddress += 64) {
        uint8_t readData[64];
        for (uint8_t i = 0; i < 64; i++) {
          readData[i] = read_byte(ramAddress+i);
        }
				// usb write
				USBD_TxData(USBD_EP_1, readData, sizeof(readData));
      }
    }
    // Disable RAM
    write_byte(0x0000, 0x00);
  }
}
// end GB/GBC

// GBA

void readHeader_GBA(int time) {
	RD(1);
	WR(1);
	CS(1);
	CS2(1);
	
	//read_word(0x0);
	for (int currWord = 0; currWord < 0xC0; currWord +=2) {
		uint16_t word = read_word(currWord);
		sdBuffer[currWord] = word & 0xFF;
		sdBuffer[currWord + 1] = (word >> 8) & 0xFF;
	}
	
	// USBD_TxData(USBD_EP_1, sdBuffer, sizeof(sdBuffer));
	
	uint16_t logoChecksum = 0;
  for (int currByte = 0x4; currByte < 0xA0; currByte++) {
    logoChecksum += sdBuffer[currByte];
  }
	
	/*
	if(logoChecksum != 0x4B1B) {
		char message[256];
		strcpy(message, "ErrorCart..........");
		USBD_TxData(USBD_EP_1, (uint8_t*)message, strlen(message));
		memset(message, 0, sizeof(message));
	} else{
	*/
		char data2send[18];
		
		for (int i = 0 ; i < 12; i++) {
			data2send[i] = sdBuffer[160+i];
			if(data2send[i] == 32) {
				data2send[i] = 95;
			}
		}
		data2send[12] = 45;
		data2send[13] = sdBuffer[172];
		data2send[14] = sdBuffer[173];
		data2send[15] = sdBuffer[174];
		data2send[16] = sdBuffer[175];
		data2send[17] = '\0';
		
		if(time == 1) {
			USBD_TxData(USBD_EP_1, (uint8_t*)data2send, strlen(data2send));
		}
		// send gameTile-Code
		memset(data2send, 0, sizeof(data2send));
		memset(sdBuffer, 0, sizeof(sdBuffer));
	//}
}

void dump_GBA(int size_type) { 
	RD(1);
	WR(1);
	CS(1);
	CS2(1);
	
	// get size
	// 1 = 1M, =2=4M, =3=8M, =4=16M, =5=32M;
	unsigned long gba_size;
	char message[256];
	switch(size_type) {
		case 1:
			gba_size = 1048576;
			break;
		
		case 2:
			gba_size = 4194304;
			break;
		
		case 3:
			gba_size = 8388608;
			break;
		
		case 4:
			gba_size = 16777216;
			break;
		
		case 5:
			gba_size = 33554432;
			break;
		
		default:
			gba_size = 0;
			break;
	}
	USBD_TxData(USBD_EP_1, (uint8_t*)message, strlen(message));
	memset(message, 0, sizeof(message));
	
	uint32_t romAddress = 0x0;
	read_word(0x0);
	while(gba_size != 0 && romAddress < gba_size) {
		uint8_t readData[64];
		for(uint8_t i = 0; i < 64; i +=2) {
			uint16_t word = read_word(romAddress+i);
			memcpy(&readData[i], &word, sizeof(uint16_t));
		}
		USBD_TxData(USBD_EP_1, readData, sizeof(readData));
		romAddress += 64;
	}
}

void readram_gba(int save_type){
}

int switchMode(void) {
	int mode = GPIO_ReadInputBit(GPIOB, GPIO_PIN_0);
	if(mode == 0) { // 0, GB/GBC mode
		config_sig_addr_gpio();
		rd_wr_mreq_reset();
		config_gpio_data_in();
		GPIO_ResetBit(GPIOC, GPIO_PIN_14);
	}
	else if(mode == 1) { //1, GBA mode
		config_sig_addr_gpio();
		config_gpio_data_out(); // GB data pins is GBA addr pins
		RD(1);
		WR(1);
		CS(1);
		CS2(1);
		GPIO_SetBit(GPIOC, GPIO_PIN_14);
	}
	nop_delay(20);
	return mode;
}

int getMode() {
	// CartType
	// 0, GB/GBC
	// 1, GBA
	int CartType = GPIO_ReadInputBit(GPIOB, GPIO_PIN_0);
	
	return CartType;
}
 
int main(void) {
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOA));
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOB));
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOC));
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOD));
	// for usb init
	config_gpio_pb5();
	GPIO_SetBit(GPIOB, GPIO_PIN_5);
	// usb cdc
	CDC_Init();
	
	// config PB0 in for get switch info
	// PB0 high = GBA Mode; PB0 low = GB/GBC Mode
	GPIO_Config_T GPIO_ConfigStruct;

	GPIO_ConfigStruct.mode = GPIO_MODE_IN_PD;
  GPIO_ConfigStruct.pin = GPIO_PIN_0;
  GPIO_ConfigStruct.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct);
	// Config PB0 end
	
	// PC14 test 
	// PC14 low->3.3v high->5v
	config_gpio_vcc();
	
	switchMode();
	while(1){
	}
}