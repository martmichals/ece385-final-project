#include "SPI.h"
#include "stdlib.h"
#include <altera_avalon_spi.h>
 #include <cstdio>

void delay(unsigned delay) {
	usleep (1000*delay);
}

unsigned long millis() {
	return clock();
}

void SPI_begin() {
}

void SPI_beginTransaction(void* arg) {;

}

void SPI_endTransaction() {
}

uint8_t SPI_transfer(uint8_t val) {

	printf("SPI_transfer\n");

	uint8_t data;

    // Write the register address, read
    int return_code = alt_avalon_spi_command(
        SPI_0_BASE, 0,      // SPI_0 base address, slave address
        1, &val,            // Write length, write data pointer
        1, &data,       // Read data, read buffer pointer
        0                   // Flags
	);

	// Check the return code
    if(return_code < 0)
        printf("Invalid return code on SPI read: %x.", return_code);

    // Return address after the last in the read buffer
	return data;
}

void SPI_transfer_multi(uint8_t* buf, unsigned len) {

	printf("SPI_transfer_multi\n");

    // Write the register address, read
    int return_code = alt_avalon_spi_command(
        SPI_0_BASE, 0,      // SPI_0 base address, slave address
        len, buf,            // Write length, write data pointer
		len, buf,       // Read data, read buffer pointer
        0                   // Flags
	);

	// Check the return code
    if(return_code < 0)
        printf("Invalid return code on SPI write: %x.", return_code);
}