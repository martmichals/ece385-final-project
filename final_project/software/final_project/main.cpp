//#include "pixel_vga.h"
#include <stdio.h>
#include <unistd.h>

#include "Discord/DiscordClient.h"
#include "VGA/pixel_vga.h"
#include "Keyboard/keyboard.h"


#define SPI_0_BASE 0x080050A0
#include <altera_avalon_spi.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
unsigned char mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xA6 };

int main() {
		init_color_palette();
		draw_background();
		draw_logo();
		init_keyboard_driver();
		clear_buffer();

		DiscordClient discordClient;
		while(1) {
			if(discordClient.fetchChannel("945536104629698631", 0)) {
				draw_rectangle(SIDEBAR_WIDTH, 32, 512, 422, 0);
			    alt_u32 y = 452 - (MESSAGE_Y_MARGIN*2) - 16;
			    for(alt_u8 i=0; i<MAX_MESSAGE_LINES; ++i) {
			    	const MessageLine* line = discordClient.getLine(i);

			    	printf(line->content);
			    	printf("\n");

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
			if (update_keyboard_driver_state()){
				draw_string(
					MESSAGE_X_MARGIN,
					454,
					(const char*) get_keyboard_buffer(),
					&fonts[UNAME_FONT]
				);
			}
		}

}

//int main() {
//		DiscordClient discordClient;
//		while(1) {
//			printf(discordClient.getServer());
//			printf("\n");
//
//			std::list<DiscordChannel>::const_iterator it;
//			for(it = discordClient.getChannels()->begin(); it != discordClient.getChannels()->end(); ++it){
//				printf("id: %s, name: %s, sidename: %s\n", it->id, it->name, it->side_name);
//			}
//			printf("\n");
//
//			discordClient.nextServer();
//			//discordClient.sendChannel("945536104629698631", "Hello from FPGA!");
//			usleep(10000000);
//		}
//
//}


//int main() {
//		init_color_palette();
//		draw_background();
//		draw_logo();
//		draw_sample();
//		alt_u8 i=0;
//		while (1) {
//			draw_channel_select((alt_u8)i%8);
//			i++;
//			usleep (100000);
//		}
//		return 1;
////	init_keyboard_driver();
////	clear_buffer();
////	while (1) {
////		if (update_keyboard_driver_state()){
////			printf("%s\n", get_keyboard_buffer());
////		}
////	}
////	return 1;
//}
//
