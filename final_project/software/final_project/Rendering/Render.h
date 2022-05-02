#ifndef RENDER_H_
#define RENDER_H_

#include "../Discord/DiscordClient.h"

class Renderer {
    public:
        // Basic functionality
        Renderer();
        ~Renderer();

        // Select the channel below or above the current
        void channel_down();
        void channel_up();

        // Selects the next or previous server
        void server_next();
        void server_prev();

        // Scroll up or down in the message history
        void scroll_up();
        void scroll_down();

        // Function to re-render the channel list on the screen
        void channel_list();

        // Function to re-render the messages on the screen
        void messages();

        // Function to re-render the server name
        void server_title();

        // Function to re-render the channel title
        void channel_title();

        // Function to render the message box
        // returns: whether the keyboard buffer should be cleared
        // (it should be cleared if a message is sent)
        // TODO don't redraw the whole thing every time
        bool update_message(const char* keyboard_buffer);

    private:
        // Discord client
        DiscordClient* client;

        // Selected channel, first channel shown, last channel shown
        char selected_channel;
        char first_channel;
        char last_channel;

        // Scroll in the messages
        uint8_t scroll;
};

#endif
