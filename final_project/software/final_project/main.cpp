//#include "pixel_vga.h"
#include <stdio.h>
#include <unistd.h>
#include "Ethernet/Ethernet.h"

#define SPI_0_BASE 0x080050A0
#include <altera_avalon_spi.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
unsigned char mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xA6 };

int main() {
	unsigned char cmd[50];
	unsigned char buf[50];

	// write to Gateway IP Address Register
	cmd[0] = 0x00;
	cmd[1] = 0x01;
	cmd[2] = 0x05;
	cmd[3] = 0xC0;

	printf("writing...");

    alt_avalon_spi_command(
        SPI_0_BASE, 0,      // SPI_0 base address, slave address
        4, cmd,            // Write length, write data pointer
		0, NULL,       // Read data, read buffer pointer
        0                   // Flags
	);

    usleep(100000);

    // read from Gateway IP Address Register
	// offset address = 0x18, RWB = 0, 0M = 0
	cmd[0] = 0x00;
	cmd[1] = 0x01;
	cmd[2] = 0x01;


    alt_avalon_spi_command(
        SPI_0_BASE, 0,      // SPI_0 base address, slave address
        3, cmd,            // Write length, write data pointer
		1, buf,       // Read data, read buffer pointer
        0                   // Flags
	);

    printf("buf: %x\n", buf[0]);

//	  if (Ethernet.begin(mac) == 0) {
//	    printf("Failed to configure Ethernet using DHCP");
//	    // Check for Ethernet hardware present
//	    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
//	      printf("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
//	      while (true) {
//	        usleep (1000); // do nothing, no point running without Ethernet hardware
//	      }
//	    }
//	    if (Ethernet.linkStatus() == LinkOFF) {
//	      printf("Ethernet cable is not connected.");
//	    }
//	  } else {
//	    printf("  DHCP assigned IP ");
//	    //printf(Ethernet.localIP());
//	  }

}

//int main() {
//	init_color_palette();
//	for(int i=1; i < 5000; i++) {
//		draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
//		usleep (100000);
//		draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DGREY);
//		usleep (100000);
//	}
//	return 1;
//}
