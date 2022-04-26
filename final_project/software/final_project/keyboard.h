#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

#include <stdio.h>
#include "system.h"
#include "altera_avalon_spi.h"
#include "altera_avalon_spi_regs.h"
#include "altera_avalon_pio_regs.h"
#include "sys/alt_irq.h"
#include "usb_kb/GenericMacros.h"
#include "usb_kb/GenericTypeDefs.h"
#include "usb_kb/HID.h"
#include "usb_kb/MAX3421E.h"
#include "usb_kb/transfer.h"
#include "usb_kb/usb_ch9.h"
#include "usb_kb/USB.h"

extern HID_DEVICE hid_device;

static BYTE addr = 1; 				//hard-wired USB address
const static char* const devclasses[] = { " Uninitialized", " HID Keyboard", " HID Mouse", " Mass storage" };

// Struct for keyboard events (really space efficient)
struct KB_EVENTS{
	alt_u8 change_server_left;
	alt_u8 change_server_right;
	alt_u8 scroll_up;
	alt_u8 scroll_down;
};

// Keyboard driver macros
#define DISCORD_CHAR_LIM 2000
#define KEYCODE_LOWER    4
#define KEYCODE_UPPER    56
#define INV              '\0'
#define SHIFT_MASK       0x22
#define TAB_WIDTH        4

// Lowercase mappings
static char* keycode_to_lower_ascii[53] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g' ,'h',
    'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z', '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '\n', INV, '\b', '\t', ' ', '-', '=', '[',
    ']', '\\', INV, ';', '\'', '`', ',', '.',  '/'
};

// Uppercase mappings (for shift)
static char* keycode_to_upper_ascii[53] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G' ,'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '\n', INV, '\b', '\t', ' ', '_', '+', '{',
    '}', '|', INV, ':', '"', '~', '<', '>',  '?'
};

// Keyboard buffer, event data structures
static alt_u32 buffer_ptr = 0;
static BYTE last_keycodes[6] = {0, 0, 0, 0, 0, 0};
static char keyboard_buffer[DISCORD_CHAR_LIM+1];
static struct KB_EVENTS keyboard_events = {0, 0, 0, 0};

alt_u8 in_last_set(BYTE keycode);
char* get_keyboard_buffer();
void clear_buffer();
int init_keyboard_driver();
int update_keyboard_driver_state();
BYTE GetDriverandReport();
void setKeycode(WORD keycode);

#endif //__KEYBOARD_H_
