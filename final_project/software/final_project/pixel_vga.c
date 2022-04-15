#include "pixel_vga.h"

void set_color_palette(alt_u8 color, alt_u8 red, alt_u8 green, alt_u8 blue) {
	// Pointer to the beginning of VRAM
    volatile alt_u32* vram_32 = (alt_u32*) vga_ctrl->VRAM;

    // Point to start of color palette
    vram_32 += PALETTE_OFFSET + (color/2);

    // color 0
    if(color % 2 == 0) 
        *vram_32 = (((alt_u32)red) << 9) | (((alt_u32)green) << 5) | (((alt_u32)blue) << 1);
    
}

void init_color_palette() {
    // Initialize the color palette with the colors declared in the struct header
    for(alt_u8 i=0; i<16; i++) {
        set_color_palette(i, colors[i].red, colors[i].green, colors[i].blue);
    }
}

void draw_rectangle(alt_u32 x, alt_u32 y, alt_u32 width, alt_u32 height, alt_u8 color){
    // Validate the arguments
    if (x%8 != 0 | width%8 != 0) {
        printf("Invalid arguments passed to draw_rectangle!\n");
    }

    // Initialize the color
    alt_u32 word = 0;
    for(alt_u8 p=0; p<8; p++) word += (color << (p*4)); 
    printf("Color: %x\n", word);

    // Iterate over the relevant words, set their color
    for(alt_u32 y_draw=y; y_draw<(y+height); y_draw++) {
        for(alt_u32 x_draw=x; x_draw<(x+width); x_draw+=8) {
            vga_ctrl->VRAM[(x_draw/8) + (y_draw*80)] = word;
        }
    }
}
