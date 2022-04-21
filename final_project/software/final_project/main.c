#include "pixel_vga.h"

int main() {
	init_color_palette();
	for(int i=1; i < 5000; i++) {
		draw_background();
		usleep (100000);
	}
	return 1;
}
