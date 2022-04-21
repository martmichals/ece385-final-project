#include "pixel_vga.h"

void set_color_palette(alt_u8 color, alt_u8 red, alt_u8 green, alt_u8 blue) {
	// Pointer to the beginning of VRAM
    volatile alt_u32* vram_32 = (alt_u32*) vga_ctrl->VRAM;

    // Point to start of color palette
    vram_32 += PALETTE_OFFSET + color;

    // Set the color
    *vram_32 = (((alt_u32)red / 16) << 8) + (((alt_u32)green / 16) << 4) + ((alt_u32)blue / 16);
}

void init_color_palette() {
    // Initialize the color palette with the colors declared in the struct header
    for(alt_u8 i=0; i<4; i++) {
        set_color_palette(i, colors[i].red, colors[i].green, colors[i].blue);
    }
}

void draw_rectangle(alt_u32 x, alt_u32 y, alt_u32 width, alt_u32 height, alt_u8 color){
    // Validate the arguments
    if (x%16 != 0 | width%16 != 0)
        printf("Invalid arguments passed to draw_rectangle!\n");

    // Initialize the color
    alt_u32 word = 0;
    for(alt_u8 p=0; p<16; p++) word += (color << (p*2)); 

    // Iterate over the relevant words, set their color
    for(alt_u32 y_draw=y; y_draw<(y+height); y_draw++) {
        for(alt_u32 x_draw=x; x_draw<(x+width); x_draw+=16) {
            vga_ctrl->VRAM[(x_draw/16) + (y_draw*40)] = word;
        }
    }
}

void debug() {
    // Clear the screen
    draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    // Set each word to a different color
    for(alt_u32 i=0; i < 100; i++){
        alt_u32 word = 0;
        for(alt_u8 p=0; p<8; p++) word += ((i%4) << (p*4)); 
        vga_ctrl->VRAM[i] = word;
    }
}

void draw_background() { 
    // Draw the sidebar background 
    draw_rectangle(0, 0, SIDEBAR_WIDTH, SCREEN_HEIGHT, 0);

    // Draw the messages background
    draw_rectangle(SIDEBAR_WIDTH, 0, SCREEN_WIDTH-SIDEBAR_WIDTH, SCREEN_HEIGHT, 0);
}
