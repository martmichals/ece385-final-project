#include <unistd.h>
#include <time.h>
typedef unsigned char uint8_t;

// SPI device address
#define SPI_0_BASE 0x080050A0

// SPI functions
unsigned long millis();
void delay(unsigned delay);

//void SPI_begin();
//void SPI_beginTransaction(void* arg);
//void SPI_endTransaction();
//
//uint8_t SPI_transfer(uint8_t val);
//void SPI_transfer_multi(uint8_t* buf, unsigned len);

