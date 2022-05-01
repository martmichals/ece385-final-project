#ifndef TEXT_MODE_VGA_COLOR_H_
#define TEXT_MODE_VGA_COLOR_H_

#include "fonts.h"
#include "logo.h"
#include <system.h>

#define CACHE_BYPASS 0x80000000

// VRAM Macros
#define SCREEN_WIDTH      640
#define SCREEN_HEIGHT     480
#define PIXELS_PER_WORD   8
#define PALETTE_OFFSET    19200
#define CONTROL_OFFSET    19204

// UI macros
#define SIDEBAR_WIDTH    128



// Memory struct
struct PIXEL_VGA_STRUCT {
	alt_u32 VRAM [(SCREEN_WIDTH*SCREEN_HEIGHT)/PIXELS_PER_WORD];
};

// Color structure
struct COLOR{
	alt_u8 red;
	alt_u8 green;
	alt_u8 blue;
};

#ifdef __cplusplus
extern "C"
{
#endif

// Base address of the VGA Pixel Controller
static volatile struct PIXEL_VGA_STRUCT* vga_ctrl = (volatile struct PIXEL_VGA_STRUCT*) (VGA_PIXEL_CONTROLLER_0_BASE | CACHE_BYPASS) ;

// CGA colors with names
static struct COLOR colors[] = {
    {0  , 0  , 0  }, // Black, does not matter
	{128, 128, 128}, // Grey
	{192, 192, 192}, // Grey-White
    {255, 255, 255}, // White
};

// Font data structure
struct FONT {
	alt_u8   height;
	alt_u8   space_pxls;
	alt_u8*  data;
	alt_u8*  widths;
	alt_u32* indices;
};

// Macros for indexing into the below list
#define SERVER_FONT  0
#define CHANNEL_FONT 1
#define TITLE_FONT   2
#define UNAME_FONT   3
#define MESSAGE_FONT 4

// List of fonts
static struct FONT fonts[] = {
	{SERVER_PIXEL_HEIGHT,  4, server_font_data,  server_widths,  server_char_indices }, // Server
	{CHANNEL_PIXEL_HEIGHT, 3, channel_font_data, channel_widths, channel_char_indices}, // Channel
	{TITLE_PIXEL_HEIGHT,   5, title_font_data,   title_widths,   title_char_indices  }, // Title
	{UNAME_PIXEL_HEIGHT,   3, uname_font_data,   uname_widths,   uname_char_indices  }, // Username
	{MESSAGE_PIXEL_HEIGHT, 3, message_font_data, message_widths, message_char_indices}  // Message
};

// ASCII decoding array
static alt_u8 decode_ascii[] = {
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
	73, 74, 75, 76, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 77, 78, 79, 80, 81, 82, 83, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49,
	50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 84, 85, 86, 87, 88, 89, 10, 11, 12, 13,
	14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
	90, 91, 92, 93, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94,
	94
};

// Rendering macros
#define SIDEBAR_X_MARGIN    5
#define SERVER_Y 			35
#define CHANNEL_Y 			52
#define CHANNEL_Y_MARGIN 	1
#define MESSAGE_X_MARGIN	SIDEBAR_WIDTH+20
#define TITLE_Y				5
#define MESSAGE_Y			34
#define UNAME_Y_MARGIN  	1
#define MESSAGE_Y_MARGIN  	1

// Declared functions
void draw_logo();
void init_color_palette();
void draw_rectangle(alt_u32 x, alt_u32 y, alt_u32 width, alt_u32 height, alt_u8 color);
void draw_char(alt_u32 x, alt_u32 y, alt_u8 render_code, struct FONT* font);
void draw_string(alt_u32 x, alt_u32 y, const char* str, struct FONT* font);
void draw_channel_select(alt_u8 channel);
void draw_background();
void draw_sample();
void debug();
#ifdef __cplusplus
}
#endif
#endif
