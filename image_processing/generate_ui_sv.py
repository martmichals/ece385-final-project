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
        'message': ('./data/fonts/uni_sans_bold.otf', 12)
    }
    for font_name in font_file_map:
        # Render the font
        font = ImageFont.truetype(font_file_map[font_name][0])
        font_img = Image.new('RGB', (640, font_file_map[font_name][1]), color=colors['grey'])
        vga_render = Image.new('RGB', (640, font_file_map[font_name][1]), color=colors['grey'])
        font_draw = ImageDraw.Draw(font_img)

        # Create a numpy array bitmap
        idx = 0
        for c in bitmaps[font_name]:
            # Numpy bitmap
            bitmap = np.zeros((font_file_map[font_name][1], 50))

            # Render the font, fill the bitmap
            font_draw.text((idx, 2), c, font=font, fill=colors['white'])
            all_black, x, y = False, idx, 0
            while not all_black:
                for y in range(0, font_file_map[font_name][1]):
                    # Set the bitmap value
                    clr = font_img.getpixel((x, y))
                    bitmap[y][x-idx] = clr[2]

                    # Set the VGA render pixel
                    vga_render.putpixel((x, y), clr if clr[2] > 0 else colors['grey'])
                all_black = (bitmap[:,x-idx].sum() == 51*font_file_map[font_name][1])
                x += 1
            idx = x

        # Save the font image
        font_img.save(f'./data/bitmaps/{font_name}.png') 
        vga_render.save(f'./data/bitmaps/{font_name}_vga.png') 

    # VGA Screen 
    # screen = [[discord_grey for _ in range(640)] for _ in range(480)]

    # # Parse the discord logo, add to the UI
    # with Image.open('./data/logo_h32.png') as logo:
    #     for x in range(logo.width):
    #         for y in range(logo.height):
    #             p_val = logo.getpixel((x,y))
    #             screen[y][x+1] = discord_blue if p_val[3] > 90 else discord_grey
    # last_logo_y = 31

    # # Output the UI Image
    # output = Image.new('RGB', (640, 480), color='black')

    # # Draw the logo on the output image
    # for y in range(len(screen)):
    #     for x in range(len(screen[0])):
    #         output.putpixel((x,y), tuple(screen[y][x]))

    # # Draw some text on the output image
    # title_font = ImageFont.truetype('./data/fonts/uni_sans_heavy.otf', 16)
    # draw_output = ImageDraw.Draw(output)
    # draw_output.text((5,last_logo_y+4), "ECE 385", font=title_font, fill=white)

    # # Draw text channel names
    # names = ['welcome', 'announcements', 'rules', 'get-roles', 'general', 'memes', 'quizzes', 'final-project']
    # channel_font = ImageFont.truetype('./data/fonts/uni_sans_regular.otf', 14)
    # for i, name in enumerate(names):
    #     draw_output.text((5,last_logo_y+4+18+(16*i)), f'# {name}', font=channel_font, fill=white)
    # for y in range(last_logo_y+4+18+(16*(len(names))), last_logo_y+4+18+(16*(len(names))+14)):
    #     for x in range(117+10):
    #         output.putpixel((x,y), white)
    # draw_output.text((5,last_logo_y+4+18+(16*len(names))), f'# off-topic', font=channel_font, fill=discord_grey)
    # more_names = ['course-matchmaking', 'scream']
    # for i, name in enumerate(more_names):
    #     draw_output.text((5,last_logo_y+4+18+(16*i)+(16*(len(names)+1))), f'# {name}', font=channel_font, fill=white)

    # # Draw overall box
    # for y in range(0, len(screen)):
    #     for x in range(117+10, len(screen[0])):
    #         output.putpixel((x,y), discord_light_grey)

    # # Draw the channel title
    # title_font = ImageFont.truetype('./data/fonts/uni_sans_heavy.otf', 24)
    # draw_output.text((117+20, 5), "# off-topic", font=title_font, fill=white)

    # # Sample names, messages
    # message_h = 12
    # username_font = ImageFont.truetype('./data/fonts/uni_sans_heavy.otf', message_h)
    # usermessage_font = ImageFont.truetype('./data/fonts/uni_sans_regular.otf', message_h)
    # usernames = ['partymarty', 'txwong2', 'martinm6', 'Captain_Sisko']
    # messages = [
    #     'I hate hardware', 'I like coding in x86. I\'m sadistic like that.', 'Why do so many people in this department just not shower?',
    #     'Who cares about the weather being nice? I\'m inside all day anyway.', 'Lol this is why I\'m not an EE major.', 'CS professors don\'t have the ability to break your soul like ECE professors' 
    # ]
    # y_off = 24+10
    # for i, message in enumerate(messages):
    #     draw_output.text((117+20, (message_h+2)*(i*2)+y_off), usernames[i%4], font=username_font, fill=white)
    #     draw_output.text((117+20, (message_h+2)*(i*2 + 1)+y_off), message, font=usermessage_font, fill=white)

    # # Text field to type in
    # for y in range(output.height-message_h-11, output.height-5):
    #     for x in range(117+20, output.width-10):
    #         output.putpixel((x,y), discord_grey)
    # draw_output.text((117+25, output.height-message_h-7), 'I don\'t enj', font=usermessage_font, fill=white)

    # Save the image
    output.save('vga_output.png')

if __name__ == '__main__':
    main()
