#include "DiscordClient.h"

// constructor
DiscordClient::DiscordClient() {
	  unsigned char mac[] = MAC_ADDRESS;

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
		client->println("Host: 149.56.141.231");
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
						if((uint8_t) messageCache[line].content[line_idx] != rx_buffer[mem_idx]) {
							changed = true;
						}
						messageCache[line].content[line_idx++] = rx_buffer[mem_idx++];
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

