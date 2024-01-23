#include <main.h>

void rd_mr_mreq_reset() {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_9, 1); // rd high to disable rd
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_8, 1); // wr high to disable wr
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_10, 1); //mreq/CS high
}

void set_sig_read() {
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_8, 1); //wr set high to disable write
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_9, 0); // rd low to enable read
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_10, 0); //mreq/CS low
}

void set_address(uint16_t address) {
	// CAN WE JUST WRITE REGISTERS? IT'S SO STUPID!
	int level;
	// PA15
	level = (address >> 15) & 1;
	GPIO_WriteBitValue(GPIOA, GPIO_PIN_15, level);
	
	// PA14
	level = (address >> 14) & 1;
	GPIO_WriteBitValue(GPIOA, GPIO_PIN_14, level);
	
	// PA13
	level = (address >> 13) & 1;
	GPIO_WriteBitValue(GPIOA, GPIO_PIN_13, level);
	
	// PC12
	level = (address >> 12) & 1;
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_12, level);
	
	// PC11
	level = (address >> 11) & 1;
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_11, level);
	
	// PA10
	level = (address >> 10) & 1;
	GPIO_WriteBitValue(GPIOA, GPIO_PIN_10, level);
	
	// PA9
	level = (address >> 9) & 1;
	GPIO_WriteBitValue(GPIOA, GPIO_PIN_9, level);
	
	// PA8
	level = (address >> 8) & 1;
	GPIO_WriteBitValue(GPIOA, GPIO_PIN_8, level);
	
	// PB7
	level = (address >> 7) & 1;
	GPIO_WriteBitValue(GPIOB, GPIO_PIN_7, level);
	
	// PB6
	level = (address >> 6) & 1;
	GPIO_WriteBitValue(GPIOB, GPIO_PIN_6, level);
	
	// PD2
	level = (address >> 5) & 1;
	GPIO_WriteBitValue(GPIOD, GPIO_PIN_2, level);
	
	// PB4
	level = (address >> 4) & 1;
	GPIO_WriteBitValue(GPIOB, GPIO_PIN_4, level);
	
	// PB3
	level = (address >> 3) & 1;
	GPIO_WriteBitValue(GPIOB, GPIO_PIN_3, level);
	
	// PB2
	level = (address >> 2) & 1;
	GPIO_WriteBitValue(GPIOB, GPIO_PIN_2, level);
	
	// PC7
	level = (address >> 1) & 1;
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_7, level);
	
	// PC6
	level = (address >> 0) & 1;
	GPIO_WriteBitValue(GPIOC, GPIO_PIN_6, level);
}

uint8_t read_byte(uint16_t address) {
	set_address(address);
	
	set_sig_read();
	// wait 3 machin circle
	__ASM volatile("nop");
	__ASM volatile("nop");
	__ASM volatile("nop");
	uint8_t bval = (GPIOB->IDATA >> 8) & 0xFF;
	
	rd_mr_mreq_reset();
	
	return bval;
}