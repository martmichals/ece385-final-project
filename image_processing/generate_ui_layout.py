from ssl import CHANNEL_BINDING_TYPES
from PIL import Image, ImageFont, ImageDraw

def main():
    # Colors
    discord_blue = (88, 101, 242)
    discord_grey = (44, 47, 51)
    discord_light_grey = (54, 57, 63)
    white = (255, 255, 255)

    # VGA Screen 
    screen = [[discord_grey for _ in range(640)] for _ in range(480)]

    # Parse the discord logo, add to the UI
    with Image.open('./data/logos/logo_h32.png') as logo:
        for x in range(logo.width):
            for y in range(logo.height):
                p_val = logo.getpixel((x,y))
                screen[y][x+1] = discord_blue if p_val[3] > 90 else discord_grey
    last_logo_y = 31

    # Output the UI Image
    output = Image.new('RGB', (640, 480), color='black')

    # Draw the logo on the output image
    for y in range(len(screen)):
        for x in range(len(screen[0])):
            output.putpixel((x,y), tuple(screen[y][x]))

    # Draw some text on the output image
    title_font = ImageFont.truetype('./data/fonts/uni_sans_heavy.otf', 16)
    draw_output = ImageDraw.Draw(output)
    draw_output.text((5,last_logo_y+4), "ECE 385", font=title_font, fill=white)

    # Draw text channel names
    names = ['l-lwelcome-', 'announcements', 'rules', 'get-roles', 'general', 'memes', 'quizzes', 'final-project']
    channel_font = ImageFont.truetype('./data/fonts/uni_sans_regular.otf', 14)
    for i, name in enumerate(names):
        print((5,last_logo_y+4+18+(16*i)))
        draw_output.text((5,last_logo_y+4+18+(16*i)), f'# {name}', font=channel_font, fill=white)
    for y in range(last_logo_y+4+18+(16*(len(names))), last_logo_y+4+18+(16*(len(names))+14)):
        for x in range(117+10):
            output.putpixel((x,y), white)
    draw_output.text((5,last_logo_y+4+18+(16*len(names))), f'# off-topic', font=channel_font, fill=discord_grey)
    more_names = ['course-matchmaking', 'scream']
    for i, name in enumerate(more_names):
        draw_output.text((5,last_logo_y+4+18+(16*i)+(16*(len(names)+1))), f'# {name}', font=channel_font, fill=white)

    # Draw overall box
    for y in range(0, len(screen)):
        for x in range(117+10, len(screen[0])):
            output.putpixel((x,y), discord_light_grey)

    # Draw the channel title
    title_font = ImageFont.truetype('./data/fonts/uni_sans_heavy.otf', 24)
    draw_output.text((117+20, 5), "# off-topic", font=title_font, fill=white)

    # Sample names, messages
    message_h = 12
    username_font = ImageFont.truetype('./data/fonts/uni_sans_heavy.otf', message_h)
    usermessage_font = ImageFont.truetype('./data/fonts/uni_sans_regular.otf', message_h)
    usernames = ['partymarty', 'txwong2', 'martinm6', 'Captain_Sisko']
    messages = [
        'I hate hardware', 'I like coding in x86. I\'m sadistic like that.', 'Why do so many people in this department just not shower?',
        'Who cares about the weather being nice? I\'m inside all day anyway.', 'Lol this is why I\'m not an EE major.', 'CS professors don\'t have the ability to break your soul like ECE professors' 
    ]
    y_off = 24+10
    for i, message in enumerate(messages):
        draw_output.text((117+20, (message_h+2)*(i*2)+y_off), usernames[i%4], font=username_font, fill=white)
        draw_output.text((117+20, (message_h+2)*(i*2 + 1)+y_off), message, font=usermessage_font, fill=white)

    # Text field to type in
    for y in range(output.height-message_h-11, output.height-5):
        for x in range(117+20, output.width-10):
            output.putpixel((x,y), discord_grey)
    draw_output.text((117+25, output.height-message_h-7), 'I don\'t enj', font=usermessage_font, fill=white)

    # Save the image
    output.save('output.png')

if __name__ == '__main__':
    main()
