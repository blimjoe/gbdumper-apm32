/* Includes */
#include "main.h"
#include "usbd_cdc.h"
#include "SEGGER_RTT.h"
#include "func.h"
#include "bsp_delay.h"
#include <string.h>

uint8_t nintendoLogo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
                          0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
                          0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
                         };

char gameTitle[17];
uint16_t cartridgeType = 0;
uint16_t romSize = 0;
uint16_t romBanks = 0;
uint16_t ramSize = 0;
uint16_t ramBanks = 0;
uint16_t ramEndAddress = 0;


void readHeader() {
	SEGGER_RTT_printf(0, "readHeader....\n");
	rd_wr_mreq_reset();
	
	for(uint16_t romAddress = 0x0134; romAddress <= 0x143; romAddress++) {
		char headerChar = (char)read_byte(romAddress);
		if ((headerChar >= 0x30 && headerChar <= 0x57) || // 0-9
			(headerChar >= 0x41 && headerChar <= 0x5A) || // A-Z
			(headerChar >= 0x61 && headerChar <= 0x7A)) { // a-z
				gameTitle[(romAddress-0x0134)] = headerChar;
				SEGGER_RTT_printf(0,"ok %d  ", headerChar);
			}
	}
	gameTitle[16] = '\0';
	for(int a=0; a<16; a++) {
		SEGGER_RTT_printf(0,"%c",gameTitle[a]);
	}

	
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
	
	SEGGER_RTT_printf(0, "GameTitle: %s\n", gameTitle);
	SEGGER_RTT_printf(0, "cartridgeType: %d\n", cartridgeType);
	SEGGER_RTT_printf(0, "romSize: %d\n", romSize);
	SEGGER_RTT_printf(0, "ramSize: %d\n", ramSize);
	SEGGER_RTT_printf(0, "logoCheck: %d\n", logoCheck);
	
	//char huanhang[1] = {10};
	USBD_TxData(USBD_EP_1, (uint8_t*)gameTitle, sizeof((uint8_t*)gameTitle));
	//USBD_TxData(USBD_EP_1, (uint8_t*)huanhang, sizeof((uint8_t*)huanhang));
}

void dumpRom(void) {
	SEGGER_RTT_printf(0,"dump rom...\n");
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
			for(uint8_t i = 0; i < 64; i++){
				readData[i] = read_byte(romAddress+i);
				// usb write
			}
			romAddress += 64;
		}
	}
	
}
 
void loop(void) {
	// get control signal from usb
	char message[20];
	uint8_t receive;
	static uint8_t dataBuf[256] = {0};
	uint8_t signal = 0; // signal from linux. default. do nothing. 1(49). read game info. 2(50). dump rom.
	
	USBD_RxData(USBD_EP_1, dataBuf, g_usbDev.outBuf[USBD_EP_1].maxPackSize);
	signal = dataBuf[0];
	memset(dataBuf, 0, sizeof(dataBuf));
	USBD_TxData(USBD_EP_1, &signal, sizeof(signal));
	if(signal != 0){
		SEGGER_RTT_printf(0, "signal = %d\n", (uint8_t)signal);
	}
	
	
	switch(signal) {
		case 49:
			readHeader();
			//strcpy(message, "case 1");
			//USBD_TxData(USBD_EP_1, (uint8_t*)message, sizeof((uint8_t*)message));
			break;
		case 50:
			dumpRom();
			strcpy(message, "case 2");
			USBD_TxData(USBD_EP_1, (uint8_t*)message, sizeof((uint8_t*)message));
			break;
		default:
			readHeader();
		/*
			strcpy(message, "Nothing\n");
			USBD_TxData(USBD_EP_1, (uint8_t*)message, sizeof((uint8_t*)message));
			memset(message, 0, sizeof(message));
		*/
			break;
	}
}
 
int main(void) {
	SEGGER_RTT_printf(0, "start now...\n");
	CDC_Init();
	config_sig_addr_gpio();
	rd_wr_mreq_reset();
	config_gpio_data_in();
	config_gpio_reset();
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_0, 1); // set reset pin low
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_14, 1);
	
	// usb cdc
	config_gpio_pb5();
	GPIO_SetBit(GPIOB, GPIO_PIN_5);
	CDC_Init();
	
	
	
	while(1) {loop();}
}