#ifndef TEXT_MODE_VGA_COLOR_H_
#define TEXT_MODE_VGA_COLOR_H_

#include <system.h>
#include <alt_types.h>

// VRAM Macros
#define SCREEN_WIDTH      640
#define SCREEN_HEIGHT     480
#define PIXELS_PER_WORD   8
#define PALETTE_OFFSET    38400

// Color macros
#define WHITE   6
#define BLACK   8
#define BLURPLE 10
#define DGREY   12 // Discord grey
#define DLGREY  14 // Discord light grey

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

// Base address of the VGA Pixel Controller
static volatile struct PIXEL_VGA_STRUCT* vga_ctrl = VGA_PIXEL_CONTROLLER_0_BASE;

// CGA colors with names
static struct COLOR colors[]={
    {64 , 64 , 64 }, // First grey
    {0  , 0  , 0  }, 
	{128, 128, 128}, // Second grey
    {0  , 0  , 0  }, 
    {192, 192, 192}, // Third grey
	{0  , 0  , 0  },
	{255, 255, 255}, // White
    {0  , 0  , 0  },
    {0  , 0  , 0  }, // Black
    {0  , 0  , 0  },
	{88 , 101, 242}, // Blurple
	{0  , 0  , 0  },
    {44 , 47 , 51 }, // Discord grey
    {0  , 0  , 0  },
	{54 , 57 , 63 }, // Discord light grey
	{0  , 0  , 0  },
};

// Declared functions
void init_color_palette();
void draw_rectangle(alt_u32 x, alt_u32 y, alt_u32 width, alt_u32 height, alt_u8 color);

#endif
