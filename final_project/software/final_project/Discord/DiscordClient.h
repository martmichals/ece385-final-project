#ifndef DISCORD_CLIENT_H_
#define DISCORD_CLIENT_H_

#define MAX_MESSAGE_LINES 25
#define MAC_ADDRESS { 0xA8, 0x61, 0x0A, 0xAE, 0x74, 0xA6 }

// can be IP or domain name
#define SERVER_ADDRESS "149.56.141.231"

#include "../Ethernet/EthernetClient.h"
#include <unistd.h>
#include <cstdlib> // malloc and free

// string at the top of /get/[channel]/[scroll]
#define DISCORD_START "DISCORD_START\n"
#define DISCORD_START_LENGTH 14

// maximum buffer size lines*(message area width / width of thinnest character) + some extra margin)
// 25*480/2 + 500
#define RX_BUFFER_SIZE 6500

// maximum length of each line, in characters
#define MAX_LINE_LENGTH 320

enum MessageFont { USERNAME, MESSAGE };

struct MessageLine {
	MessageFont font;
	char content[MAX_LINE_LENGTH];
};

class DiscordClient {

public:
  DiscordClient();
  ~DiscordClient();
  // initializes the Ethernet client
  void init();

  // blocking function that populates the cache with the contents of a channel
  // returns true if there was an update, false otherwise
  bool fetchChannel(const char* channelId, uint8_t scroll);

  // gets a specified line
  // line 0 is at the bottom, line MAX_MESSAGE_LINES is at the top
  const MessageLine* getLine(uint8_t line);

  // blocking function that sends a message using the bot account
  void sendChannel(const char* channelId, const char* message);


private:
  int discordStartPos(const char* buffer, unsigned len);

  MessageLine messageCache[MAX_MESSAGE_LINES];
  EthernetClient* client;

  // ensures fetchChannel always returns true on the first call
  bool first_run = true;
};

#endif