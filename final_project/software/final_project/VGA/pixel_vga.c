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

void draw_char(alt_u32 x, alt_u32 y, alt_u8 render_code, struct FONT* font) {
    // Get the character metadata
    alt_u8  char_width = font->widths[render_code];
    alt_u8  char_height = font->height;
    alt_u32 font_outer = font->indices[render_code];
    alt_u8  font_inner = 0;

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
                alt_u8 font_data = font->data[font_outer];

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
}

void draw_string(alt_u32 x, alt_u32 y, const char* str, struct FONT* font) {
    // Find the length of the string in pixels
    alt_u32 str_char_len = 0;
    for (alt_u32 i=0; str[i] != '\0'; i++) str_char_len++;

    // Character-relevant data
    alt_u32 font_outers[str_char_len];
    alt_u8  font_inners[str_char_len];

    // Get pixel boundaries for the characters
    alt_u32 str_pxl_len = 0;
    alt_u32 pxl_boundaries[str_char_len];
    for (alt_u32 i=0; str[i] != '\0'; i++) {
        alt_u8 render_code = decode_ascii[str[i]];

        // Set pixel boundaries
        if(str[i] == ' ') str_pxl_len += font->space_pxls;
        else str_pxl_len += font->widths[render_code];
        pxl_boundaries[i] = str_pxl_len;

        // Set font pointers and metadata
        font_outers[i]    = font->indices[render_code];
        font_inners[i]    = 0;
    }

    // Font metadata
    alt_u32 char_idx    = 0;
    alt_u8  char_height = font->height;

    // Iterate over the relevant VRAM words
    // x_wb, y_wb are guaranteed to point to the beginning of a relevant word of memory
    alt_u32 start_wb_x = x-(x%16);
    for(alt_u32 y_wb=y; y_wb<(y+char_height); y_wb++) {
        for(alt_u32 x_wb=start_wb_x; x_wb<(x+str_pxl_len); x_wb+=16) {

            // Iterate through all of the pixels in the word
            alt_u32 wy = y_wb;
            alt_u32 word = 0;
            for(alt_u32 wx=x_wb; wx < x_wb+16; wx++) {
                // Check if the pixel is outside the character to be drawn
                if (wx < x) continue;

                // Check if we are out of the width of the string
                if (wx-x >= str_pxl_len) continue;

                // Figure out which character we are drawing
                if (wx-x >= pxl_boundaries[char_idx]) char_idx++;

                // Get the relevant character from the font data
                alt_u8 font_data = font->data[font_outers[char_idx]];

                // Extract the relevant pixel data
                alt_u32 pixel_data = (font_data >> ((3-font_inners[char_idx])*2)) % 4;
                if (str[char_idx] == ' ') pixel_data = 0;

                // Write the pixel data to the word
                word += pixel_data << (15-(wx%16))*2;

                // Increment the pixel counter for the font data
                if (++font_inners[char_idx] == 4){
                    font_inners[char_idx] = 0;
                    font_outers[char_idx]++;
                }

            // Write the word to VRAM
            vga_ctrl->VRAM[(x_wb/16) + (y_wb*40)] = word;
            }
        }
        // Reset the character index for the next row of pixels
        char_idx = 0;
    }
}

void draw_sample() {
    // Draw the server name
    draw_string(SIDEBAR_X_MARGIN, SERVER_Y, "ECE 385", &fonts[SERVER_FONT]);

    // Draw the channel names
    const char channels[8][255] = {
        "#welcome", "#news", "#rules", "#get-roles",
        "#general", "#memes", "#quizzes", "#final-project"
    };
    for(alt_u8 i=0; i<8; i++) {
        draw_string(
            SIDEBAR_X_MARGIN,
            (i*fonts[CHANNEL_FONT].height) + (i*CHANNEL_Y_MARGIN*2) + CHANNEL_Y_MARGIN + CHANNEL_Y,
            channels[i],
            &fonts[CHANNEL_FONT]
        );
    }

    // Draw the title for the message board
    draw_string(MESSAGE_X_MARGIN, TITLE_Y, "#general", &fonts[TITLE_FONT]);

    // Usernames, messages
    const char unames[4][255] = {
        "partymarty", "Captain_Sisko", "txwong2", "martinm6"
    };
    const char messages[6][255] = {
        "I hate hardware",
        "I like coding in x86. I'm sadistic like that.",
        "Why do so many people in this department just not shower?",
        "Who cares about the weather being nice? I'm inside all day anyway.",
        "Lol this is why I'm not an EE major.",
        "CS professors don't have the ability to break your soul like ECE professors"
    };
    alt_u32 y = MESSAGE_Y;
    for(alt_u8 i=0; i<6; i++) {
        // Draw the uname
        draw_string(
            MESSAGE_X_MARGIN,
            y + UNAME_Y_MARGIN,
            unames[i%4],
            &fonts[UNAME_FONT]
        );
        y += fonts[UNAME_FONT].height + 2*UNAME_Y_MARGIN;

        // Draw the message
        draw_string(
            MESSAGE_X_MARGIN,
            y + MESSAGE_Y_MARGIN,
            messages[i],
            &fonts[MESSAGE_FONT]
        );
        y += fonts[MESSAGE_FONT].height + 2*MESSAGE_Y_MARGIN;
    }
}

void draw_channel_select(alt_u8 channel){
     vga_ctrl->VRAM[CONTROL_OFFSET] = (((alt_u32)channel)*fonts[CHANNEL_FONT].height) + (((alt_u32)channel)*CHANNEL_Y_MARGIN*2) + CHANNEL_Y;
}

alt_u32 interleave(alt_u32 x) {
    // Code from https://stackoverflow.com/questions/39490345/interleave-bits-efficiently
    alt_u32 B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
    alt_u32 S[] = {1, 2, 4, 8};

    x = (x | (x << S[3])) & B[3];
    x = (x | (x << S[2])) & B[2];
    x = (x | (x << S[1])) & B[1];
    x = (x | (x << S[0])) & B[0];

    return x | (x << 1);
}

void draw_logo() {
    for(alt_u32 i=0; i < 128; i++) {
        // 16 MSBs
        alt_u32 top = logo[i] >> 16;
        vga_ctrl->VRAM[(i/4)*40 + ((i%4)*2)]   = interleave(top);

        // 16 LSBs
        alt_u32 bottom = logo[i] & 0xFFFF;
        vga_ctrl->VRAM[(i/4)*40 + ((i%4)*2)+1] = interleave(bottom);
    }
}