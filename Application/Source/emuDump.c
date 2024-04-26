#include "main.h"
#include "usbd_cdc.h"
#include "func.h"

void emuGB(int startAddr, int length) {
	rd_wr_mreq_reset();
	
	uint16_t romAddress = (uint16_t)startAddr;
	while(romAddress < startAddr + (uint16_t)length) {
		uint8_t readData[64];
		for(uint8_t i = 0; i < 64; i++){
				readData[i] = read_byte(romAddress+i);
		}
		USBD_TxData(USBD_EP_1, readData, sizeof(readData));
		romAddress += 64;
	}
}