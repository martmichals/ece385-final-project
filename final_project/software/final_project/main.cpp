//#include "pixel_vga.h"
#include <stdio.h>
#include <unistd.h>

#include "Discord/DiscordClient.h"
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

		DiscordClient discordClient;
		while(1) {
			if(discordClient.fetchChannel("945536104629698631", 0)) {
				draw_rectangle(128, 32, 512, 240, 0);
			    alt_u32 y = 226;
			    for(alt_u8 i=0; i<12; ++i) {
			    	const MessageLine* line = discordClient.getLine(i);

			    	if(line->font == USERNAME) {
				        // Draw the uname
				        draw_string(
				            MESSAGE_X_MARGIN,
				            y + UNAME_Y_MARGIN,
				            line->content,
				            &fonts[UNAME_FONT]
				        );
				        y -= 16;
			    	} else if (line->font == MESSAGE) {
				        // Draw the message
				        draw_string(
				            MESSAGE_X_MARGIN,
				            y + MESSAGE_Y_MARGIN,
							line->content,
				            &fonts[MESSAGE_FONT]
				        );
				        y -= 16;
			    	}
			    }
			}
		}

}




//int main() {
//	init_color_palette();
//	draw_background();
//	draw_sample();
//	return 1;
//}
