#include "pixel_vga.h"

int main() {
	init_color_palette();
	draw_background();
	draw_logo();
	draw_sample();
	alt_u8 i=0;
	while (1) {
		draw_channel_select((alt_u8)i%8);
		i++;
		usleep (100000);
	}
	return 1;
}
