#include "pixel_vga.h"

int main() {
	init_color_palette();
	draw_background();
	draw_sample();
	usleep (1000000);
	return 1;
}
