#include <main.h>
#include "SEGGER_RTT.h"
#include <usbd_core.h>
#include <usbd_cdc.h>
#include <func.h>
#include <string.h>
#include <pin_out.h>

void debug_log(char log[256]){
	char message[256];
	strcpy(message,log);
	USBD_TxData(USBD_EP_1, (uint8_t*)message, strlen(message)+1);
	memset(message, 0, sizeof(message));
}

void config_sig_addr_gpio(void) {
	GPIO_Config_T GPIO_ConfigStruct_A;
	GPIO_Config_T GPIO_ConfigStruct_B;
	GPIO_Config_T GPIO_ConfigStruct_C;
	GPIO_Config_T GPIO_ConfigStruct_D;
	
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOA));
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOB));
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOC));
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOD));
	
	// GPIOA Group PA8/PA9/PA10/PA13/PA14/PA15
	GPIO_ConfigStruct_A.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_A.pin = GPIO_PIN_8	| GPIO_PIN_9	| GPIO_PIN_10 | 
														GPIO_PIN_13 | GPIO_PIN_14	| GPIO_PIN_15;
  GPIO_ConfigStruct_A.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOA, &GPIO_ConfigStruct_A);
	
	RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_AFIO);
	//GPIO_ConfigPinRemap(GPIO_REMAP_SWJ_JTAGDISABLE);
	GPIO_ConfigPinRemap(GPIO_REMAP_SWJ_DISABLE);
	
	// GPIOB Group PB2/PB3/PB4/PB6/PB7
	GPIO_ConfigStruct_B.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_B.pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | 
														GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_ConfigStruct_B.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct_B);
	
	// GPIOC Group PC6/PC7/PC11/PC12  SIG_PIN PC8/PC9/PC10/PC0
	GPIO_ConfigStruct_C.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_C.pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 |
														GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | 
														GPIO_PIN_12 | GPIO_PIN_0;
  GPIO_ConfigStruct_C.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOC, &GPIO_ConfigStruct_C);
	
	// GPIOD Group PD2
	GPIO_ConfigStruct_D.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct_D.pin = GPIO_PIN_2;
  GPIO_ConfigStruct_D.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOD, &GPIO_ConfigStruct_D);
}

void config_gpio_data_out(void) {
	GPIO_Config_T GPIO_ConfigStruct;
	
	// GPIOB PB8...15
	GPIO_ConfigStruct.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct.pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
													GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_ConfigStruct.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct);
}

void config_gpio_data_in(void) {
	GPIO_Config_T GPIO_ConfigStruct;
	
	// GPIOB PB8...15
	GPIO_ConfigStruct.mode = GPIO_MODE_IN_PD;
  GPIO_ConfigStruct.pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 |
													GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_ConfigStruct.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct);
}

void config_gpio_vcc(void) {
	GPIO_Config_T GPIO_ConfigStruct;

	GPIO_ConfigStruct.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct.pin = GPIO_PIN_14;
  GPIO_ConfigStruct.speed = GPIO_SPEED_2MHz;
  GPIO_Config(GPIOC, &GPIO_ConfigStruct);
}

void config_gpio_pb5(void) {
	GPIO_Config_T GPIO_ConfigStruct;
	
	RCM_EnableAPB2PeriphClock((RCM_APB2_PERIPH_T)(RCM_APB2_PERIPH_GPIOB));
	GPIO_ConfigStruct.mode = GPIO_MODE_OUT_PP;
  GPIO_ConfigStruct.pin = GPIO_PIN_5;
  GPIO_ConfigStruct.speed = GPIO_SPEED_50MHz;
  GPIO_Config(GPIOB, &GPIO_ConfigStruct);
}

void rd_wr_mreq_reset() {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_9, 1); // rd high to disable rd
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_8, 1); // wr high to disable wr
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_10, 1); //mreq/CS high
}

void CS(int level) {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_10, level);
}

void CS2(int level) {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_0, level);
}

void WR(int level) {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_8, level);
}

void RD(int level) {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_9, level);
}

void set_sig_read() {
	WR(1); //wr set high to disable write
	RD(0); // rd low to enable read
	CS(0); //mreq/CS low
}

void set_address(uint16_t address) {
	int level;
	for (int i = 15; i >= 0; i--) {
		level = (address >> i) & 1;
		GPIO_WriteBitValue(address_pin[i].gpiox, address_pin[i].pin, level);
	}
}

void set_address_gba(unsigned long address) {
	int level;
	
}


uint8_t read_byte(uint16_t address) {
	set_address(address);
	set_sig_read();

	// wait 3 machin circle
	
	__ASM volatile("nop");
	__ASM volatile("nop");
	__ASM volatile("nop");
	
	uint8_t bval = 0;
	
	for(int i = 7; i >= 0; i--) {
		int level = GPIO_ReadInputBit(data_pin[i].gpiox, data_pin[i].pin);
		bval = bval | level << i;
	}
	rd_wr_mreq_reset();
	return bval;
}


void write_byte(uint16_t address, uint8_t data){
	int level;
	char message[256];

	config_gpio_data_out();
	set_address(address);
	
	// reset data pins
	for(int a = 7; a >= 0; a--) {
		GPIO_ResetBit(data_pin[a].gpiox, data_pin[a].pin);
	}
	
	// set data to data pins
	for(int i = 7; i >= 0; i--) {
		level = (data >> i) & 1;
		GPIO_WriteBitValue(data_pin[i].gpiox, data_pin[i].pin, level);
	}

	// wr set low to enable
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_8, 0);
	__ASM volatile("nop");
	__ASM volatile("nop");
	__ASM volatile("nop");
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_8, 1);
	
	config_gpio_data_in();
}

void EXIT0_IRQHandler(void) {
	
}

