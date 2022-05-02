#include "Render.h"
#include "../VGA/pixel_vga.h"

// Constructor
Renderer::Renderer() {
    // Initialize the discord client
    client = new DiscordClient();

    // Wipe the screen, draw the Discord logo
    init_color_palette();
    draw_background();
    draw_logo();

    // Render the server name
    server_title();

    // Render channel list
    selected_channel = 0;
    first_channel = 0;
    last_channel = 20;
    channel_list();

    // Render message list
    scroll = 0;
    messages();

    // Render channel title
    channel_title();
}

// Destructor
Renderer::~Renderer() {
    delete client;
}

// Render the channel list
void Renderer::channel_list() {
    // Get a pointer to the channel list
    const std::list<DiscordChannel>* channels = client->getChannels();
    std::list<DiscordChannel>::const_iterator it = channels->begin();

    // Adjust render bounds if necessary
    if (selected_channel <= first_channel) {
        char delta = first_channel - selected_channel;
        first_channel -= delta;
        last_channel -= delta;
    } else if (selected_channel >= last_channel) {
        char delta = selected_channel - last_channel;
        first_channel += delta;
        last_channel += delta;
    }

    // Set the control register to select the requested channel
    draw_channel_select(selected_channel-first_channel);

    // Render the channels
    for(char i=0; i <= last_channel && it != channels->end(); i++) {
        if (i >= first_channel) {
            draw_string(
                SIDEBAR_X_MARGIN,
                (((int)(i-first_channel)) * fonts[CHANNEL_FONT].height) + (((int)(i-first_channel)) * CHANNEL_Y_MARGIN * 2) + CHANNEL_Y_MARGIN + CHANNEL_Y,
                (const char*) it->side_name,
                &fonts[CHANNEL_FONT]
            );
        }
        ++it;
    }
}

// Render the messages on the screen
void Renderer::messages() {
    // Get all the messages for the selected channel
    const std::list<DiscordChannel>* channels = client->getChannels();
    std::list<DiscordChannel>::const_iterator it = channels->begin();

    // Iterate to the channel of interest
    for(char i=0; i != selected_channel; i++) ++it;


    // Render channel of interest
    if (client->fetchChannel((const char*) it->id, scroll)) {
        // Clear out messages
        draw_rectangle(SIDEBAR_WIDTH, 32, 512, 422, 0);

        // Draw the messages to the screen
        alt_u32 y = 452 - (MESSAGE_Y_MARGIN*2) - 16;
        for(alt_u8 i=0; i<MAX_MESSAGE_LINES; ++i) {
            const MessageLine* line = client->getLine(i);
            if (line->font == USERNAME) {
                // Draw the uname
                draw_string(
                    MESSAGE_X_MARGIN,
                    y + UNAME_Y_MARGIN,
                    line->content,
                    &fonts[UNAME_FONT]
                );
                y -= 16;
            } else if (line->font == MESSAGE) {
                // Draw the message
                draw_string(
                    MESSAGE_X_MARGIN,
                    y + MESSAGE_Y_MARGIN,
                    line->content,
                    &fonts[MESSAGE_FONT]
                );
                y -= 16;
            }
        }
    }
}

void Renderer::server_title() {
    // Draw the server name
    draw_string(SIDEBAR_X_MARGIN, SERVER_Y, client->getServer(), &fonts[SERVER_FONT]);
}


void Renderer::channel_title() {
    // Get all the messages for the selected channel
    const std::list<DiscordChannel>* channels = client->getChannels();
    std::list<DiscordChannel>::const_iterator it = channels->begin();

    // Iterate to the channel of interest
    for(char i=0; i != selected_channel; i++) ++it;

    // Draw the channel name
    draw_rectangle(SIDEBAR_WIDTH, 0, 512, 32, 0);
    draw_string(MESSAGE_X_MARGIN, TITLE_Y, it->name, &fonts[TITLE_FONT]);
}

bool Renderer::update_message(const char* keyboard_buffer) {

	// draw message box background
    draw_rectangle(SIDEBAR_WIDTH, 452, SCREEN_WIDTH-SIDEBAR_WIDTH, 28, 0);

    // if there's a trailing newline
    if(keyboard_buffer[strlen(keyboard_buffer)-1] == '\n') {
        // Get all the messages for the selected channel
        const std::list<DiscordChannel>* channels = client->getChannels();
        std::list<DiscordChannel>::const_iterator it = channels->begin();

        // Iterate to the channel of interest
        for(char i=0; i != selected_channel; i++) ++it;

        // send the keyboard buffer to the proper channel
    	client->sendChannel(it->id, keyboard_buffer);

    	// clear the buffer
    	return true;
    }

    // if there is not a trailing newline...

    // draw message text
	draw_string(
		MESSAGE_X_MARGIN,
		454,
		keyboard_buffer,
		&fonts[UNAME_FONT]
	);

	return false;
}

void Renderer::channel_down() {
    // Set the appropriate state
    scroll = 0;

    // Increment the selected channel, if still within bounds
    const std::list<DiscordChannel>* channels = client->getChannels();
    if (selected_channel < channels->size()-1) {
        selected_channel++;
    }

    // Change the viewing window, if necessary, rerender
    if (selected_channel == last_channel+1) {
        first_channel++;
        last_channel++;

        // Clear the sidebar
        draw_rectangle(0, 32, SIDEBAR_WIDTH, SCREEN_HEIGHT-32, 0);

        // Render all elemnents of the screen
        server_title();
        channel_list();
        channel_title();
        messages();
        return;
    }

    // Skip the channel list re-render
    channel_title();
    messages();
    draw_channel_select(selected_channel-first_channel);
}

void Renderer::channel_up() {
    // Set the appropriate state
    scroll = 0;

    // Increment the selected channel, if still within bounds
    const std::list<DiscordChannel>* channels = client->getChannels();
    if (selected_channel > 0) {
        selected_channel--;
    }

    // Change the viewing window, if necessary, rerender
    if (selected_channel == first_channel-1) {
        first_channel--;
        last_channel--;

        // Clear the sidebar
        draw_rectangle(0, 32, SIDEBAR_WIDTH, SCREEN_HEIGHT-32, 0);

        // Render all elements of the screen other than server title
        channel_list();
        channel_title();
        messages();
    } else {
        // Skip the channel list re-render
        channel_title();
        messages();
        draw_channel_select(selected_channel-first_channel);
        return;
    }
}

void Renderer::scroll_down() {
    // Check bounds
    if (scroll > 0) scroll--;

    // Render messages
    messages();
}

void Renderer::scroll_up() {
    // Increment scroll
    scroll++;


    // Render messages
    messages();
}

void Renderer::server_next() {
	client->nextServer();

    // Wipe the screen, draw the Discord logo
    draw_background();
    draw_logo();

    // Render the server name
    server_title();

    // Render channel list
    selected_channel = 0;
    first_channel = 0;
    last_channel = 20;
    channel_list();

    // Render message list
    scroll = 0;
    messages();

    // Render channel title
    channel_title();
}

void Renderer::server_prev() {
	client->prevServer();

    // Wipe the screen, draw the Discord logo
    draw_background();
    draw_logo();

    // Render the server name
    server_title();

    // Render channel list
    selected_channel = 0;
    first_channel = 0;
    last_channel = 20;
    channel_list();

    // Render message list
    scroll = 0;
    messages();

    // Render channel title
    channel_title();
}
