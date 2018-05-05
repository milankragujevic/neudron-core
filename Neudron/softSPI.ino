#define NOP() __asm__ __volatile__("nop")

void Read_Packet(uint8_t *data, uint8_t length) {
	uint8_t i;
	CS_off;
	spi_write(0x61);
	for (i=0;i<length;i++) {
		data[i]=spi_read();
	}
	CS_on;
}

uint8_t spi_write(uint8_t command) {
	uint8_t result=0;
	uint8_t n=8;
	SCK_off;
	MOSI_off;
	while(n--) {
		if(command & 0x80) {
			MOSI_on;
		} else {
			MOSI_off;
		}
		if(MISO_on) {
			result |= 0x01;
		}
		SCK_on;
		NOP();
		SCK_off;
		command = command << 1;
		result = result << 1;
	}
	MOSI_on;
	return result;
}

void spi_write_address(uint8_t address, uint8_t data) {
	CS_off;
	spi_write(address);
	NOP();
	spi_write(data);
	CS_on;
}

uint8_t spi_read() {
	uint8_t result=0;
	uint8_t i;
	MOSI_off;
	NOP();
	for(i=0;i<8;i++) {
		if(MISO_on) {
			result = (result<<1)|0x01;
		} else {
			result = result<<1;
		}
		SCK_on;
		NOP();
		SCK_off;
		NOP();
	}
	return result;
}

uint8_t spi_read_address(uint8_t address) {
	uint8_t result;
	CS_off;
	spi_write(address);
	result = spi_read();
	CS_on;
	return(result);
}
