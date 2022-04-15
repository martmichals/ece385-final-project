#include "pixel_vga.h"

int main() {
	init_color_palette();
	for(int i=1; i < 5000; i++) {
		draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
		usleep (100000);
		draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, DGREY);
		usleep (100000);
	}
	return 1;
}
