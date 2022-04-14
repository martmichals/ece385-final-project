import string
import numpy as np
from PIL import Image, ImageFont, ImageDraw

def main():
    # Colors
    colors = {
        'blurple'    : (88, 101, 242),
        'grey'       : (44, 47, 51),
        'light_grey' : (54, 57, 63),
        'white'      : (255, 255, 255)
    }

    # Wipe the macros file, write colors
    with open('./data/sv_files/macros.sv', 'w') as f:
        f.write('// Color macros\n')
        for color in colors:
            macro_val = '{ '
            for i, val in enumerate(colors[color]):
                macro_val += f'8\'d{val}' if i == 2 else f'8\'d{val}, '
            macro_val += ' }'
            f.write(f'`define {color.upper()}{" "*(12-len(color))}{macro_val}\n')
        f.write('\n')

    # Macros for logo rendering
    LOGO_START_Y = 0
    LOGO_END_Y   = 31
    LOGO_START_X = 1
    LOGO_END_X   = 117

    # Macros for server title font
    FONT_SERVER_YMARGIN = 1
    FONT_SERVER_HEIGHT  = 16

    # Macros for channel font
    FONT_CHANNEL_YMARGIN = 1
    FONT_CHANNEL_HEIGHT  = 14

    # Macros for sidebar rendering
    SIDEBAR_SERVER_Y_OFFSET  = LOGO_END_Y + 4 + FONT_SERVER_YMARGIN
    SIDEBAR_CHANNEL_Y_OFFSET = LOGO_END_Y + FONT_SERVER_YMARGIN*2 + FONT_SERVER_HEIGHT
    SIDEBAR_OFFSET_X         = 5
    SIDEBAR_END_X            = LOGO_END_X + 10

    # Write important y-axis bounds to the macros file
    with open('./data/sv_files/macros.sv', 'a') as f:
        f.write('// Logo macros\n')
        f.write(f'`define LOGO_START_Y    {LOGO_START_Y}\n')
        f.write(f'`define LOGO_END_Y      {LOGO_END_Y}\n')
        f.write(f'`define LOGO_START_X    {LOGO_START_X}\n')
        f.write(f'`define LOGO_END_X      {LOGO_END_X}\n')
        f.write('\n')
        f.write('// Server font macros\n')
        f.write(f'`define FONT_SERVER_YMARGIN    {FONT_SERVER_YMARGIN}\n')
        f.write(f'`define FONT_SERVER_HEIGHT     {FONT_SERVER_HEIGHT}\n')
        f.write('\n')
        f.write('// Channel font macros\n')
        f.write(f'`define FONT_CHANNEL_YMARGIN   {FONT_CHANNEL_YMARGIN}\n')
        f.write(f'`define FONT_CHANNEL_HEIGHT    {FONT_CHANNEL_HEIGHT}\n')
        f.write('\n')
        f.write('// Sidebar macros\n')
        f.write(f'`define SIDEBAR_SERVER_Y_OFFSET   {SIDEBAR_SERVER_Y_OFFSET}\n')
        f.write(f'`define SIDEBAR_CHANNEL_Y_OFFSET  {SIDEBAR_CHANNEL_Y_OFFSET}\n')
        f.write(f'`define SIDEBAR_OFFSET_X          {SIDEBAR_OFFSET_X}\n')
        f.write(f'`define SIDEBAR_END_X             {SIDEBAR_END_X}\n')
        f.write('\n')

    # Create a dictionary for all displayable characters in each font
    rom_char_order = string.printable
    bitmaps = {
        f: {c: None for c in rom_char_order} for f in ['server', 'channel', 'title', 'uname', 'message']
    }
    font_file_map = {
        'server': ('./data/fonts/uni_sans_heavy.otf', 14),
        'channel': ('./data/fonts/uni_sans_regular.otf', 16),
        'title': ('./data/fonts/uni_sans_bold.otf', 24),
        'uname': ('./data/fonts/uni_sans_bold.otf', 12),
        'message': ('./data/fonts/uni_sans_regular.otf', 12)
    }
    for font_name in font_file_map:
        # Render the font
        font = ImageFont.truetype(font_file_map[font_name][0], font_file_map[font_name][1])
        font_img = Image.new('RGB', (2000, font_file_map[font_name][1]+20), color=colors['grey'])
        vga_render = Image.new('RGB', (2000, font_file_map[font_name][1]+20), color=colors['grey'])
        font_draw = ImageDraw.Draw(font_img)

        # Create a numpy array bitmap
        idx = 0
        for c in bitmaps[font_name]:
            # Numpy bitmap
            bitmap = np.full((font_file_map[font_name][1]+20, 200), 51)

            # Render the font, fill the bitmap
            div = 64
            font_draw.text((idx, 2), c, font=font, fill=colors['white'])
            all_black, x, y = False, idx, 0
            while not all_black:
                for y in range(0, font_file_map[font_name][1]+20):
                    # Set the bitmap value
                    clr = font_img.getpixel((x, y))
                    bitmap[y][x-idx] = clr[2] if clr == colors['grey'] else (int(sum(clr) / 3) // div)

                    # Set the VGA render pixel
                    clr_out = (int(sum(clr) / 3) // div) * div + 64
                    vga_render.putpixel((x, y), (clr_out, clr_out, clr_out) if clr[2] != 51 else colors['light_grey'])
                all_black = (bitmap[:,x-idx].sum() == 51*(font_file_map[font_name][1]+20)) and x-idx > 1
                x += 1
            idx = x

            # Save the bitmap to the bitmap dictionary
            bitmaps[font_name][c] = np.copy(bitmap)

        # Save the font image
        font_img.save(f'./data/bitmaps/{font_name}.png') 
        vga_render.save(f'./data/bitmaps/{font_name}_vga.png') 

    # Convert each of the font bitmap dictionaries to a single np array
    for b, font_name in enumerate(bitmaps):
        # Slice all bitmaps into uniform height matrices
        char_bitmaps = [bitmaps[font_name][c] for c in rom_char_order]
        max_toff, max_boff = np.inf, np.inf
        for i, bitmap in enumerate(char_bitmaps):
            toff, boff, roff = 0, 1, 1
            while np.all((bitmap[toff] == np.array([51]*bitmap.shape[1]))) and toff < 10: toff += 1
            while np.all((bitmap[bitmap.shape[0]-boff] == np.array([51]*bitmap.shape[1]))) and boff < 20: boff += 1
            while np.all((bitmap[:,(bitmap.shape[1]-roff)] == np.array([51]*bitmap.shape[0]))) and roff < bitmap.shape[1]: roff += 1
            max_toff, max_boff = min(max_toff, toff), min(max_boff, boff)
            char_bitmaps[i] = bitmap[:,0:bitmap.shape[1]-roff+2]
        for i, bitmap in enumerate(char_bitmaps):
            char_bitmaps[i] = bitmap[max_toff:bitmap.shape[0]-max_boff+1]
            for y in range(char_bitmaps[i].shape[0]):
                for x in range(char_bitmaps[i].shape[1]):
                    if char_bitmaps[i][y, x] == 51:
                        char_bitmaps[i][y, x] = 0
        
        # Drop the last 5 characters
        char_bitmaps = char_bitmaps[:-5]

        # Get the max width, height of chars
        max_width, height = np.array([m.shape[1] for m in char_bitmaps]).max(), np.array([m.shape[0] for m in char_bitmaps]).max()
        print(font_name, 'w:', max_width, 'h:', height, 'charset:', len(char_bitmaps))

        # Validate the maps
        idx = 0
        bitmap_render = Image.new('RGB', (np.array([m.shape[1] for m in char_bitmaps]).sum(), height), color=colors['grey'])
        for i, bitmap in enumerate(char_bitmaps):
            for y in range(height):
                for x in range(bitmap.shape[1]):
                    clr = int(bitmap[y][x]) * div + 64
                    bitmap_render.putpixel((x+idx, y), (clr, clr, clr))
            idx+=x
        bitmap_render.save(f'./data/bitmaps/{font_name}_bitmap.png')

        # Convert the bitmaps into a C data structure
        with open('./data/c_fonts/fonts.c', 'w' if b==0 else 'a') as f:
            f.write(f'// Font data for {font_name} font, autogenerated from Python script\n')
            bytes_required = np.array([(m.shape[1]*height*2) // 8 + 1  for m in char_bitmaps]).sum()
            f.write(f'char {font_name}_font_data[{bytes_required}] = {{\n') 
            byte, byte_pointers, widths = 0, [], []
            # Write the font data, store index pointers, along with widths
            for i, m in enumerate(char_bitmaps):
                char, bslash = rom_char_order[i], '\\'
                cdata, curr_char = '    ', []
                byte_pointers.append(byte)
                widths.append(m.shape[1])
                for val in m.ravel():
                    curr_char.append(val)
                    if len(curr_char) == 4:
                        cdata += '0x{:02x}'.format((32*curr_char[1])+(16*curr_char[1])+(4*curr_char[1])+curr_char[0]) + ', '
                        byte += 1
                        curr_char = []
                while len(curr_char) != 4:
                    curr_char.append(0)
                cdata += '0x{:02x}'.format((32*curr_char[1])+(16*curr_char[1])+(4*curr_char[1])+curr_char[0]) + ','
                byte += 1
                f.write(f'{cdata}{" "*(910-len(cdata))} // {char if char != bslash else "bslash"}\n')
            print(f'{font_name}: {byte} bytes written')
            f.write('};\n') 
            # Write the index pointers, widths
            f.write(f'uint_32 {font_name}_char_indices[{len(byte_pointers)}] = {{{", ".join([str(ptr) for ptr in byte_pointers])}}};\n')
            f.write(f'char {font_name}_widths[{len(widths)}] = {{{", ".join([str(width) for width in widths])}}};\n\n')
            

        # Write macros for each font 
        with open('./data/c_fonts/fonts.h', 'w' if b==0 else 'a') as f:
            f.write(f'// Macros for {font_name} font, autogenerated from Python script\n')
            f.write(f'define {font_name.upper()}_PIXEL_HEIGHT{" "*(12-len(font_name))} {height}\n\n')



if __name__ == '__main__':
    main()
