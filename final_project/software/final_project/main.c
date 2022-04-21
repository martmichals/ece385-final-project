#include "pixel_vga.h"

int main() {
	init_color_palette();
	draw_background();
	draw_char(48, 48, 0);
	draw_char(48, 10, 1);
	draw_char(48, 100, 2);
	draw_char(100, 100, 20);
	draw_char(100, 40, 24);
	usleep (1000000);
	return 1;
}
