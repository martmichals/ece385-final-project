#include "pixel_vga.h"
#include <unistd.h>
#include "Ethernet/Ethernet.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
unsigned char mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xA6 };

int main() {
	  if (Ethernet.begin(mac) == 0) {
	    printf("Failed to configure Ethernet using DHCP");
	    // Check for Ethernet hardware present
	    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
	      printf("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
	      while (true) {
	        usleep (1000); // do nothing, no point running without Ethernet hardware
	      }
	    }
	    if (Ethernet.linkStatus() == LinkOFF) {
	      printf("Ethernet cable is not connected.");
	    }
	  } else {
	    printf("  DHCP assigned IP ");
	    //printf(Ethernet.localIP());
	  }

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