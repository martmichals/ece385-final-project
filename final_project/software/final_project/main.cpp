//#include "pixel_vga.h"
#include <stdio.h>
#include <unistd.h>
#include "Ethernet/Ethernet.h"

#include "pixel_vga.h"

#define SPI_0_BASE 0x080050A0
#include <altera_avalon_spi.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
unsigned char mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xA6 };

int main() {
		init_color_palette();
		draw_background();
		draw_sample();
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
	    printf("DHCP assigned IP: ");
	    Ethernet.localIP().print();
	    printf("\n");
	  }

	  EthernetClient client;
	  char server[] = "149.56.141.231";    // name address for Google (using DNS)

	  // keep polling the web server
	  while(1) {
		  if (client.connect(server, 80)) {
		  	    //printf("connected to ");
		  	    //client.remoteIP().print();
		  	    //printf("\n");
		  	    // Make a HTTP request:
		  	    client.println("GET / HTTP/1.1");
		  	    client.println("Host: 149.56.141.231");
		  	    client.println("Connection: close");
		  	    client.println();

		  	  } else {
		  	    // if you didn't get a connection to the server:
		  		  printf("connection failed");
		  	  }
		  	  unsigned long byteCount = 0;

		  	  while(1) {
		  		  // if there are incoming bytes available
		  		  // from the server, read them and print them:
		  		  int len = client.available();
		  		  uint8_t buffer[400];
		  		  if (len > 0) {

		  		    //printf("Original len: %d", len);
		  		    if (len > 400) len = 400;
		  		    client.read(buffer, len);
//		  		    for (int i = 0; i < len; ++i) {
//		  		      printf("%c", buffer[i]); // show in the serial monitor (slows some boards)
//		  		    }
		  		    byteCount = byteCount + len;
		  		  }

		  		  // if the server's disconnected, stop the client:
		  		  if (!client.connected()) {
//		  		    printf("\n");
//		  		    printf("disconnecting.");
//		  		    client.stop();
//		  		    printf("Received ");
//		  		    printf("%u", byteCount);
//		  		    printf(" bytes");
//		  		    printf("\n");

		  			char* start = (char*) buffer;

		  			while(!isDiscord(++start));
		  			start += 16;
		  			buffer[len] = '\0';
		  			//start += 16;
		  			printf(start);

		  			draw_string(MESSAGE_X_MARGIN, 100, start, &fonts[MESSAGE_FONT]);

		  			usleep (100000);
		  		    break;
		  		  }
		  }
	  }

}




//int main() {
//	init_color_palette();
//	draw_background();
//	draw_sample();
//	usleep (1000000);
//	return 1;
//}
