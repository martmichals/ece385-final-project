#include "pixel_vga.h"
#include "keyboard.h"

int main() {
	init_keyboard_driver();
	clear_buffer();
	while (1) {
		if (update_keyboard_driver_state()){
			printf("%s\n", get_keyboard_buffer());
		}
	}
	return 1;
}
