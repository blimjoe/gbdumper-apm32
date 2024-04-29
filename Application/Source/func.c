#include <main.h>
#include "SEGGER_RTT.h"
#include <usbd_core.h>
#include <usbd_cdc.h>
#include <func.h>
#include <string.h>
#include <pin_out.h>
#include <stdio.h>


extern void asm_set_address(uint16_t address);

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
		GPIO_Config_T  configStruct;

    RCM_EnableAPB2PeriphClock(RCM_APB2_PERIPH_GPIOC);
		RCM_ConfigHSE(RCM_HSE_CLOSE);

    configStruct.pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    configStruct.mode = GPIO_MODE_OUT_PP;
    configStruct.speed = GPIO_SPEED_2MHz;
    GPIO_Config(GPIOC, &configStruct);
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
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_10, 0); //mreq/CS low
}

void CS(int level) {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_10, level);
}

void CS2(int level) {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_0, !level);
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


// GPIO CFG Register
void shift_gba_direction_16(int mode) {
	uint32_t a = GPIOA->CFGHIG;
	uint32_t b = GPIOB->CFGLOW;
	uint32_t c_l = GPIOC->CFGLOW;
	uint32_t c_h = GPIOC->CFGHIG;
	uint32_t d = GPIOD->CFGLOW;
	
	// clear bits
	a &= ~(0xfff00fff);
	b &= ~(0xff0fff00);
	c_l &= ~(0xff000000);
	c_h &= ~(0xff000);
	d &= ~(0xf00);
	
	// set bit
	if (mode) {
		a |= 0x88800888;
		b |= 0x88088800;
		c_l |= 0x88000000;
		c_h |= 0x88000;
		d |= 0x800;
	} else {
		a |= 0x33300333;
		b |= 0x33033300;
		c_l |= 0x33000000;
		c_h |= 0x33000;
		d |= 0x300;
	}
	
	GPIOA->CFGHIG = a;
	GPIOB->CFGLOW = b;
	GPIOC->CFGLOW = c_l;
	GPIOC->CFGHIG = c_h;
	GPIOD->CFGLOW = d;
}

void set_address(uint16_t address) {
	uint16_t a = GPIO_ReadOutputPort(GPIOA);
	uint16_t b = GPIO_ReadOutputPort(GPIOB);
	uint16_t c = GPIO_ReadOutputPort(GPIOC);
	uint16_t d = GPIO_ReadOutputPort(GPIOD);
	
	
	// clear origin bits
	a &= ~(0b1110011100000000);
	b &= ~(0b0000000011011100);
	c &= ~(0b0001100011000000);
	d &= ~(0b0000000000000100);
	
	// set bit
	a |= (address & 0b1110011100000000);
	b |= (address & 0b0000000011011100);
	c |= (address & 0b0001100000000000);
	c |= ((address & 0b0000000000000011) << 6);
  d |= ((address & 0b0000000000100000) >> 3);
	

	GPIO_WriteOutputPort(GPIOA, a);
	GPIO_WriteOutputPort(GPIOB, b);
	GPIO_WriteOutputPort(GPIOC, c);
	GPIO_WriteOutputPort(GPIOD, d);
}

void set_address_gba(uint32_t address) {

	uint16_t a = GPIO_ReadOutputPort(GPIOA);
	uint16_t b = GPIO_ReadOutputPort(GPIOB);
	uint16_t c = GPIO_ReadOutputPort(GPIOC);
	uint16_t d = GPIO_ReadOutputPort(GPIOD);
	
	// clear origin bits
	a &= ~(0b1110011100000000);
	b &= ~(0b1111111111011100);
	c &= ~(0b0001100011000000);
	d &= ~(0b0000000000000100);
	
	// set bit
	a |= (address & 0b1110011100000000);
	b |= (address & 0b0000000011011100);
	b |= ((address & 0b1111111100000000) >> 8);
	c |= (address & 0b0001100000000000);
	c |= ((address & 0b0000000000000011) << 6);
  d |= ((address & 0b0000000000100000) >> 3);
	

	GPIO_WriteOutputPort(GPIOA, a);
	GPIO_WriteOutputPort(GPIOB, b);
	GPIO_WriteOutputPort(GPIOC, c);
	GPIO_WriteOutputPort(GPIOD, d);
}

void nop_delay(int times) {
	for (int i = 0; i < times; i++) {
		__ASM volatile("nop");
	}
}

uint8_t read_byte(uint16_t address) {
	set_address(address);
	set_sig_read();
	
	uint8_t bval = 0;
	
	uint16_t b = GPIO_ReadInputPort(GPIOB);
	bval |= ((b & 0xff00) >> 8);
	
	rd_wr_mreq_reset();
	return bval;
}

uint16_t read_word(uint32_t address) {
	address = address >> 1;
	WR(1);
	RD(1);
	CS(1);
	
  set_address_gba(address);
	__ASM volatile("nop");
	
  CS(0);
	shift_gba_direction_16(1);
	RD(0);

  __ASM volatile("nop");
  uint16_t wval = 0;
	
	uint16_t a = GPIO_ReadInputPort(GPIOA);
	uint16_t b = GPIO_ReadInputPort(GPIOB);
	uint16_t c = GPIO_ReadInputPort(GPIOC);
	uint16_t d = GPIO_ReadInputPort(GPIOD);
	
	wval |= ((c & 0xC0) >> 6);
	wval |= b & 0x1C;
	wval |= ((d & 0x4) << 3);
	wval |= b & 0xC0;
	wval |= a & 0x700;
	wval |= c & 0x1800;
	wval |= a & 0xE000;

	//extern void switchMode(int mode);
	//switchMode(1);
	RD(1);
	shift_gba_direction_16(0);
	
	return wval;
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

