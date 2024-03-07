#include <main.h>


void config_sig_addr_gpio(void);
void config_gpio_data_out(void);
void config_gpio_data_in(void);
void config_gpio_gba_keys(void);
void config_gpio_vcc(void);
void rd_wr_mreq_reset(void);
void config_gpio_pb5(void);
uint8_t read_byte(uint16_t address);
void write_byte(uint16_t address, uint8_t data);

struct GPIOX {
	GPIO_T * gpiox;
	uint16_t pin;
};

