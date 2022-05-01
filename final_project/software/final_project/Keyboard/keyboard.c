#include "keyboard.h"

BYTE GetDriverandReport() {
	BYTE i;
	BYTE rcode;
	BYTE device = 0xFF;
	BYTE tmpbyte;

	DEV_RECORD* tpl_ptr;
	printf("Reached USB_STATE_RUNNING (0x40)\n");
	for (i = 1; i < USB_NUMDEVICES; i++) {
		tpl_ptr = GetDevtable(i);
		if (tpl_ptr->epinfo != NULL) {
			printf("Device: %d", i);
			printf("%s \n", devclasses[tpl_ptr->devclass]);
			device = tpl_ptr->devclass;
		}
	}
	//Query rate and protocol
	rcode = XferGetIdle(addr, 0, hid_device.interface, 0, &tmpbyte);
	if (rcode) {   //error handling
		printf("GetIdle Error. Error code: ");
		printf("%x \n", rcode);
	} else {
		printf("Update rate: ");
		printf("%x \n", tmpbyte);
	}
	printf("Protocol: ");
	rcode = XferGetProto(addr, 0, hid_device.interface, &tmpbyte);
	if (rcode) {   //error handling
		printf("GetProto Error. Error code ");
		printf("%x \n", rcode);
	} else {
		printf("%d \n", tmpbyte);
	}
	return device;
}

void setKeycode(WORD keycode) {
	//IOWR_ALTERA_AVALON_PIO_DATA(0x8004000, keycode);
	IOWR_ALTERA_AVALON_PIO_DATA(SPI_0_BASE, keycode); // TODO: Change this to support eth & keyboard
}

int init_keyboard_driver() {
	BYTE rcode;
	BOOT_MOUSE_REPORT buf;		//USB mouse report
	BOOT_KBD_REPORT kbdbuf;

	BYTE runningdebugflag = 0;  //flag to dump out a bunch of information when we first get to USB_STATE_RUNNING
	BYTE errorflag = 0;         //flag once we get an error device so we don't keep dumping out state info
	BYTE device;
	WORD keycode;

	printf("Initializing MAX3421E...\n");
	MAX3421E_init();
	printf("Initializing USB...\n");
	USB_init();
	while (1) {
		printf(".");
		MAX3421E_Task();
		USB_Task();
		//usleep (500000);
		if (GetUsbTaskState() == USB_STATE_RUNNING) {
			if (!runningdebugflag) {
				runningdebugflag = 1;
				device = GetDriverandReport();
			} else if (device == 1) {
				//run keyboard debug polling
				rcode = kbdPoll(&kbdbuf);
				if (rcode == hrNAK) {
					continue; //NAK means no new data
				} else if (rcode) {
					printf("Rcode: ");
					printf("%x \n", rcode);
					continue;
				}
				printf("keycodes: ");
				for (int i = 0; i < 6; i++) {
					printf("%x ", kbdbuf.keycode[i]);
				}
				setKeycode(kbdbuf.keycode[0]);
				printf("\n");
				printf("USB Keyboard Successfully Initialized!\n");
				break;
			}
		} else if (GetUsbTaskState() == USB_STATE_ERROR) {
			if (!errorflag) {
				errorflag = 1;
				printf("USB Error State\n");
			}
		} else {
			printf("USB task state: ");
			printf("%x\n", GetUsbTaskState());
			if (runningdebugflag) {	//previously running, reset USB hardware just to clear out any funky state, HS/FS etc
				runningdebugflag = 0;
				MAX3421E_init();
				USB_init();
			}
			errorflag = 0;
		}

	}
	return 0;
}

// Get the keyboard buffer
char* get_keyboard_buffer() {
	return keyboard_buffer;
}

// Clear the buffer
void clear_buffer() {
	for(alt_u32 i=0; i < DISCORD_CHAR_LIM+1; i++) keyboard_buffer[i] = '\0';
	buffer_ptr=0;
	for(alt_u8 i=0; i < 6; i++)
		last_keycodes[i] = 0;
}

// Check if a key was released
alt_u8 in_last_set(BYTE keycode) {
	for(alt_u8 i=0; i < 6; i++) {
		if (keycode == last_keycodes[i])
			return 1;
	}
	return 0;
}

int process_keystroke(BYTE keycode, BOOT_KBD_REPORT* kbdbuf) {
	if ((keycode >= KEYCODE_LOWER) & (keycode <= KEYCODE_UPPER)) {
		// Populate the buffer
		if (buffer_ptr < DISCORD_CHAR_LIM) {
			// Check shift being depressed
			if (kbdbuf->mod & SHIFT_MASK) {
				keyboard_buffer[buffer_ptr] = keycode_to_upper_ascii[keycode-KEYCODE_LOWER];
			} else {
				keyboard_buffer[buffer_ptr] = keycode_to_lower_ascii[keycode-KEYCODE_LOWER];
			}
		}

		// Invalid keycode
		if (keyboard_buffer[buffer_ptr] == '\0') {
			return 0;
		}

		// Backspace
		if (keyboard_buffer[buffer_ptr] == '\b') {
			keyboard_buffer[buffer_ptr] = '\0';
			if(buffer_ptr > 0) {
				buffer_ptr--;
				keyboard_buffer[buffer_ptr] = '\0';
			}
			return 1;
		}

		// Tab
		if (keyboard_buffer[buffer_ptr] == '\t') {
			for (alt_u8 i=0; i<TAB_WIDTH & buffer_ptr<DISCORD_CHAR_LIM; i++) {
				keyboard_buffer[buffer_ptr] = ' ';
				buffer_ptr++;
			}
			return 1;
		}

		// Normal char
		buffer_ptr++;
		return 1;

		// TODO: Control logic
	}
	return 0;
}

// Returns 1 on new data
int update_keyboard_driver_state() {
	// Check USB run state
	MAX3421E_Task();
	USB_Task();
	if (!(GetUsbTaskState() == USB_STATE_RUNNING)) {
		init_keyboard_driver();
		return 0;
	}

	// Grab rcode and data if available
	BYTE rcode;
	BOOT_KBD_REPORT kbdbuf;

	// Get keyboard data
	rcode = kbdPoll(&kbdbuf);
	if (rcode == hrNAK) {
		for(alt_u8 i=0; i < 6; i++) {
			last_keycodes[i] = 0;
		}
		return 0;
	} else if (rcode) {
		return 0;
	}

	// Grab the most recent keycode, convert to ascii
	int status = 0;
	for (alt_u8 i=0; i<6; i++) {
		if (!in_last_set(kbdbuf.keycode[i]))
			status += process_keystroke(kbdbuf.keycode[i], &kbdbuf);
	}
	for(alt_u8 i=0; i < 6; i++)
		last_keycodes[i] = kbdbuf.keycode[i];

	return status > 0;
}