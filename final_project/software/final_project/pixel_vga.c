#include "fonts.h"
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

// TODO: Create structures for the fonts
void draw_char(alt_u8 x, alt_u8 y, alt_u8 render_code) {
    // Get the character metadata
    alt_u8 char_width = title_widths[render_code];
    alt_u8 char_height = TITLE_PIXEL_HEIGHT;
    alt_u32 font_outer = title_char_indices[render_code];

    // Debug
    printf("Drawing character with render code: %d\n", render_code);
    printf("Character has height:               %d\n", char_height);
    printf("Character has width:                %d\n", char_width);
    printf("Character has index into data array:%d\n", font_outer);

    // Index for character data
    alt_u8 font_inner = 0;

    // Iterate over the relevant VRAM words
    // x_wb, y_wb are guaranteed to point to the beginning of a relevant word of memory
    alt_u32 start_wb_x = x-(x%16);
    for(alt_u32 y_wb=y; y_wb<(y+char_height); y_wb++) {
        for(alt_u32 x_wb=start_wb_x; x_wb<(x+char_width); x_wb+=16) {

            // Iterate through all of the pixels in the word
            alt_u32 wy = y_wb;
            alt_u32 word = 0;
            for(alt_u32 wx=x_wb; wx < x_wb+16; wx++) {
                // Check if the pixel is outside the character to be drawn
                if (wx < x) continue;

                // Check if we are out of the width of the font
                if (wx-x >= char_width) continue;

                // Get the relevant character from the font data
                alt_u8 font_data = title_font_data[font_outer];

                // Extract the relevant pixel data
                alt_u32 pixel_data = (font_data >> ((3-font_inner)*2)) % 4;

                // Write the pixel data to the word
                word += pixel_data << (15-(wx%16))*2;

                // Increment the pixel counter for the font data
                if (++font_inner == 4){
                    font_inner = 0;
                    font_outer++;
                } 

            // Write the word to VRAM
            vga_ctrl->VRAM[(x_wb/16) + (y_wb*40)] = word;
            }
        }
    }

    // Iterate over VRAM words
    // for(alt_u32 y_draw=y; y_draw<(y+height); y_draw++) {
    //     for(alt_u32 x_draw=x; x_draw<(x+width); x_draw+=16) {
    //         // Construct the word with the font data
    //         alt_u32 word = 0;
            
    //         // Iterate through the relevant pixels
    //         for (alt_u8 data_x=x_draw-x; data_x < (16-(x_draw%16)) & data_x < width; data_x++) {
    //             // Get relevant data
    //             alt_u8 font_data = title_font_data[char_data_idx];
    //             font_data = (font_data >> ((3-char_data_inner_idx)*2)) % 4;

    //             // Populate word
    //             word += (((alt_u32)font_data) << ((15-((data_x+x)%16))*2));

    //             // Increment indices into the font data structure
    //             if(++char_data_inner_idx == 4) {
    //                 char_data_inner_idx = 0;
    //                 char_data_idx++;
    //             }
    //         }

    //         // Write the word to VRAM
    //         vga_ctrl->VRAM[(x_draw/16) + (y_draw*40)] = word;
    //     }
    // }
}
