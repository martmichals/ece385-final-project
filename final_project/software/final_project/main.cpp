#include <stdio.h>
#include <unistd.h>

//#include "Discord/DiscordClient.h"
//#include "VGA/pixel_vga.h"
#include "Keyboard/keyboard.h"
#include "Rendering/Render.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
unsigned char mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xA6 };

int main() {
	Renderer render;

	// better ece 385 bot spam channel
	render.server_next();
	render.channel_down();


	while (1) {
		if (update_keyboard_driver_state()){
			if(render.update_message((const char*) get_keyboard_buffer())) {
				clear_buffer();
			}
		}
		render.messages();
	}
	return 0;
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
//
//			if(discordClient.fetchChannel("945536104629698631", 0)) {
//				for(alt_u8 i=0; i<MAX_MESSAGE_LINES; ++i) {
//					const MessageLine* line = discordClient.getLine(i);
//
//					printf(line->content);
//					printf("\n");
//				}
//			}
//
//
//			usleep(1000000);
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
//	init_keyboard_driver();
//	while (1) {
//		if (update_keyboard_driver_state()){
//			printf("%s\n", get_keyboard_buffer());
//		}
//	}
//	return 1;
//}

