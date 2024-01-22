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

