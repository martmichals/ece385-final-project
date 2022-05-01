#include "DiscordClient.h"

// constructor
DiscordClient::DiscordClient() {
	  unsigned char mac[] = MAC_ADDRESS;

	  // initialize local variables
	  first_run = true;
	  current_idx = 0;

	if (Ethernet.begin(mac) == 0) {
		printf("Failed to configure Ethernet using DHCP");
		// Check for Ethernet hardware present
		if (Ethernet.hardwareStatus() == EthernetNoHardware) {
			printf("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
			while (true) usleep(100000); // do nothing
		}
		if (Ethernet.linkStatus() == LinkOFF) {
			printf("Ethernet cable is not connected.");
		}
	} else {
		printf("DHCP assigned IP: ");
		Ethernet.localIP().print();
		printf("\n");
	}

	client = new EthernetClient();
}

// destructor
DiscordClient::~DiscordClient() {
	// free the EthernetClient instance
	delete client;
}

bool DiscordClient::fetchChannel(const char* channelId, uint8_t scroll) {
	const char* server = SERVER_ADDRESS;

	if (client->connect(server, 80)) {
		//printf("connected to ");
		//client.remoteIP().print();
		//printf("\n");
		// Make a HTTP request:
		//client->print("GET /get/945536104629698631/0 HTTP/1.1");
		char scroll_str[4];
		sprintf(scroll_str, "%u", scroll);

		client->print("GET /get/");
		client->print(channelId);
		client->print("/");
		client->print((const char*)scroll_str);
		client->println(" HTTP/1.1");
		client->print("Host: ");
		client->println(SERVER_ADDRESS);
		client->println("Connection: close");
		client->println();

	} else {
		// if you didn't get a connection to the server:
		printf("connection failed.\n");
	}

	// allocate an rx buffer
	uint8_t* rx_buffer = (uint8_t*) malloc(RX_BUFFER_SIZE);

	// keeps track of where the buffer ends, resets once DISCORD_START is found
	unsigned buffer_end = 0;

	while (1) {
	    unsigned len = client->available();


		if (len > 0) {

			// read 100 bytes at a time
			if (len > 100)
				len = 100;

			// TODO handle this edge case (or just increase rx buffer size if needed)
			// this is here to avoid possible segfaults
			if(buffer_end + len >= RX_BUFFER_SIZE) {
				printf("Server sent too much data! Channel has not been fetched.\n");
				printf("Either the headers are too big or DISCORD_START is not present in the server response.\n");
				free(rx_buffer);
				return false;
			}

			// read the data
			client->read(rx_buffer + buffer_end, len);

			//printf("\nNew Buffer:\n");
			//for (unsigned i = 0; i < buffer_end + len; ++i) {
				//printf("%c", rx_buffer[i]);
			//}

			buffer_end += len;

			// if the buffer contains the Discord start string, reset the buffer end point to overwrite the header data
			int discord_start = discordStartPos((const char*) rx_buffer, buffer_end);
			if(discord_start > 0) {
				// move the data after the starting point to the beginning of the buffer
				for(unsigned i = 0; i < buffer_end-discord_start; ++i) {
					rx_buffer[i] = rx_buffer[i+discord_start];
				}

				// shift the buffer endpoint
				buffer_end -= discord_start;
			}

			//printf("\ndiscordStartPos: %i\n", discordStartPos((const char*) rx_buffer, buffer_end));

		}

		// if the server's disconnected, stop the client:
		if (!client->connected()) {
			client->stop();

			// track if the text has changed
			bool changed = false;

			unsigned mem_idx = 0;
			for(int line = 0; line < MAX_MESSAGE_LINES; ++line) {
				bool first = true;
				unsigned line_idx = 0;

				// for each line, loop until newline
				while(rx_buffer[mem_idx] != '\n' && line_idx < MAX_LINE_LENGTH && mem_idx < buffer_end) {
					if(first) {
						if(rx_buffer[mem_idx] == 'u') {
							if(messageCache[line].font == MESSAGE) changed = true;
							messageCache[line].font = USERNAME;
						} else if (rx_buffer[mem_idx] == 'm') {
							if(messageCache[line].font == USERNAME) changed = true;
							messageCache[line].font = MESSAGE;
						} else {
							printf("Warning: line %i does not have a valid start character!\n", line);
							// just put a blank line
							messageCache[line].font = MESSAGE;
							break;
						}
						first = false;
						++mem_idx;
					// for lines after first, copy the buffer into the line and increment the memory addresses
					} else {

						// must cast from char to uint8_t or this test will fail in certain edge cases
						// (specifically, with character codes >=128 such as the unknown character)
						// ignore special characters when computing change
						if((uint8_t) messageCache[line].content[line_idx] != rx_buffer[mem_idx] && !(rx_buffer[mem_idx] & 0x80)) {
							changed = true;
						}

						//TODO figure out why special characters are split in two
						if(rx_buffer[mem_idx] == 0xc3) {
							messageCache[line].content[line_idx++] = 0xff;
							++mem_idx;
						}
						else if (rx_buffer[mem_idx] & 0x80) {
							++mem_idx;
						} else {
							messageCache[line].content[line_idx++] = rx_buffer[mem_idx++];
						}

					}

				}
				// move past the trailing newline
				++mem_idx;

				// null-terminate the end of the string
				messageCache[line].content[line_idx] = '\0';

				// print message content
				//printf("Line: %i, Font: %i, Content: %s\n", line, messageCache[line].font, messageCache[line].content);

				// print out warnings
				if(line_idx >= MAX_LINE_LENGTH) {
					printf("WARNING: Received a line that was too long!\n");
					free(rx_buffer);
					return false;
				}
				if(mem_idx >= buffer_end && line < MAX_MESSAGE_LINES-1) {
					printf("WARNING: Reached end of RX buffer while reading Discord messages!\n");
					free(rx_buffer);
					return false;
				}

			}

			// free the read buffer and return
			free(rx_buffer);

			changed = changed || first_run;
			first_run = false;
			return changed;
		}
	}
}


const char* DiscordClient::getServer() {
	const char* server = SERVER_ADDRESS;

	if (client->connect(server, 80)) {
		char idx_str[4];
		sprintf(idx_str, "%u", current_idx);

		client->print("GET /server/");
		client->print(idx_str);
		client->println(" HTTP/1.1");
		client->print("Host: ");
		client->println(SERVER_ADDRESS);
		client->println("Connection: close");
		client->println();

	} else {
		// if you didn't get a connection to the server:
		printf("connection failed.\n");
	}

	// allocate an rx buffer
	uint8_t* rx_buffer = (uint8_t*) malloc(RX_BUFFER_SIZE);

	// keeps track of where the buffer ends, resets once DISCORD_START is found
	unsigned buffer_end = 0;

	while (1) {
	    unsigned len = client->available();


		if (len > 0) {

			// read 100 bytes at a time
			if (len > 100)
				len = 100;

			// TODO handle this edge case (or just increase rx buffer size if needed)
			// this is here to avoid possible segfaults
			if(buffer_end + len >= RX_BUFFER_SIZE) {
				printf("Server sent too much data! Channel has not been fetched.\n");
				printf("Either the headers are too big or DISCORD_START is not present in the server response.\n");
				free(rx_buffer);
				return "NULL";
			}

			// read the data
			client->read(rx_buffer + buffer_end, len);

			//printf("\nNew Buffer:\n");
			//for (unsigned i = 0; i < buffer_end + len; ++i) {
				//printf("%c", rx_buffer[i]);
			//}

			buffer_end += len;

			// if the buffer contains the Discord start string, reset the buffer end point to overwrite the header data
			int discord_start = discordStartPos((const char*) rx_buffer, buffer_end);
			if(discord_start > 0) {
				// move the data after the starting point to the beginning of the buffer
				for(unsigned i = 0; i < buffer_end-discord_start; ++i) {
					rx_buffer[i] = rx_buffer[i+discord_start];
				}

				// shift the buffer endpoint
				buffer_end -= discord_start;
			}

			//printf("\ndiscordStartPos: %i\n", discordStartPos((const char*) rx_buffer, buffer_end));

		}

		// if the server's disconnected, stop the client:
		if (!client->connected()) {
			client->stop();

			// clear the existing channel list
			discordChannels.clear();

			unsigned mem_idx = 0;

			// set the server name to the first line of the rx buffer
			while(rx_buffer[mem_idx] != '\n' && mem_idx < MAX_NAME_LENGTH && mem_idx < buffer_end) {
				serverName[mem_idx] = rx_buffer[mem_idx];
				++mem_idx;
			}
			// null terminate the server name
			serverName[mem_idx] = '\0';

			// increment mem_idx so it starts after the newline
			++mem_idx;

			// keep going until the end of the buffer
			while(mem_idx < buffer_end) {
				// the response from the server is formatted in groups of 3 lines
				// line 0: channel id
				// line 1: channel title name
				// line 2: channel sidebar name

				// declare a new discord channel
				DiscordChannel chan;

				// the response from the server is formatted in groups of 3 lines
				// line 0: channel id
				// line 1: channel title name
				// line 2: channel sidebar name

				for(unsigned line = 0; line < 3; ++line) {
					// loop through the line to populate the data
					unsigned line_idx = 0;
					while(rx_buffer[mem_idx] != '\n' && line_idx < MAX_LINE_LENGTH && mem_idx < buffer_end) {
						if(line == 0) {
							chan.id[line_idx] = rx_buffer[mem_idx];
						} else if (line == 1) {

							//TODO figure out why special characters are split in two
							if(rx_buffer[mem_idx] == 0xc3) {
								chan.name[line_idx] = 0xff;
							} else if (rx_buffer[mem_idx] & 0x80) {
								// don't add the extra character
								--line_idx;
							} else {
								chan.name[line_idx] = rx_buffer[mem_idx];
							}
						} else if (line == 2) {

							//TODO figure out why special characters are split in two
							if(rx_buffer[mem_idx] == 0xc3) {
								chan.side_name[line_idx] = 0xff;
							} else if (rx_buffer[mem_idx] & 0x80) {
								// don't add the extra character
								--line_idx;
							} else {
								chan.side_name[line_idx] = rx_buffer[mem_idx];
							}
						}
						// move to the next character in the line
						++mem_idx;
						++line_idx;
					}
					// null terminate the channel strings
					if(line == 0) {
						chan.id[line_idx] = '\0';
					} else if (line == 1) {
						chan.name[line_idx] = '\0';
					} else if (line == 2) {
						chan.side_name[line_idx] = '\0';
					}

					if(line_idx >= MAX_LINE_LENGTH) {
						printf("WARNING: Received a line that was too long while reading channels!\n");
						free(rx_buffer);
						return "NULL";
					}

					// remove the trailing newline from the buffer
					++mem_idx;
				}

				// print channel content
				//printf("id: %s, name: %s, sidename: %s\n", chan.id, chan.name, chan.side_name);

				// add to the channel list
				discordChannels.push_back(chan);

			}

			// free the read buffer and return
			free(rx_buffer);

			return serverName;
		}
	}
}

void DiscordClient::nextServer() {
	current_idx = (current_idx+1)%MAX_SERVERS;
}

void DiscordClient::prevServer() {
	current_idx = (current_idx-1)%MAX_SERVERS;
}

const std::list<DiscordChannel>* DiscordClient::getChannels() {
	return &discordChannels;
}

// searches a character buffer for DISCORD_START
// returns the index of the first character after DISCORD_START
// if DISCORD_START is not found, this returns -1
int DiscordClient::discordStartPos(const char* buffer, unsigned len) {
	const char* target = DISCORD_START;
	const unsigned target_length = DISCORD_START_LENGTH;

	// linear substring search (assumine target_length is constant)

	for(unsigned start = 0; start < len - target_length; ++start) {
		bool equal = true;
		for(unsigned i = 0; i < target_length; ++i) {
			if(target[i] != buffer[start+i]) {
				equal = false;
				break;
			}
		}
		// if the string is found, return the next character position
		if(equal) {
			return start + target_length;
		}
	}
	return -1;
}

// returns a pointer to a given message line
const MessageLine* DiscordClient::getLine(uint8_t line) {
	return &messageCache[line];
}

void DiscordClient::sendChannel(const char* channelId, const char* message) {
	const char* server = SERVER_ADDRESS;
	const unsigned length = strlen(message);

	if (client->connect(server, 80)) {
		char length_str[4];
		sprintf(length_str, "%u", length);

		// send the headers
		client->print("POST /send/");
		client->print(channelId);
		client->println(" HTTP/1.1");
		client->print("Host: ");
		client->println(SERVER_ADDRESS);
		client->println("Content-Type: text/plain");
		client->print("Content-Length: ");
		client->println(length_str);
		client->println("Connection: close");
		client->println();

		// send the data
		client->print(message);

		// stop the client
		client->stop();

	} else {
		// if you didn't get a connection to the server:
		printf("connection failed.\n");
	}
}

