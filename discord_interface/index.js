const secret = require('./secret').secret;
const stringf = require('./stringf');

const express = require('express');
const app = express();
const port = 3000;

const { Client, Intents } = require('discord.js');
const client = new Client({ intents: [Intents.FLAGS.GUILDS, Intents.FLAGS.GUILD_MESSAGES] });

let lastMessage = '';

// servers is a dict of objects
// key: server id
// each object has properties id, name, and textchannels

// textchannels is a dict of objects
// key: channel id
// each object has id, name, position, messagess

// messages is a list of objects
// each object has a nickname, timestamp, and content

let servers = {};

let initialized = false;

client.on('ready', async () => {
	console.log(`Logged in as ${client.user.tag}!`);

    console.log('Generating list of servers...');

    const guilds = await client.guilds.fetch();
    for(const guild of guilds) {
        console.log(`Setting up ${guild[1].name}...`);

        const channels = await (await guild[1].fetch()).channels.fetch();

        let channelList = [];
        for(const channel of channels) {
            console.log(`Caching channel #${channel[1].name}...`);
            if(channel[1].type === 'GUILD_TEXT') {

                // try to fetch the last 20 messages of each text channel
                let messages = null;

                try {
                    messages = await channel[1].messages.fetch({ limit: 20 });
                } catch (DiscordAPIError) {
                    console.warn(`Could not fetch message history for #${channel[1].name} in ${guild[1].name}`);
                    continue;
                }

                let messageList = [];
                for(const message of messages) {
                    // ommit blank messages
                    // if(message[1].content === '') continue;

                    messageList.push({
                        id: message[1].id,
                        nickname: message[1].member === null ? message[1].author.username : message[1].member.displayName,
                        timestamp: message[1].createdTimestamp,
                        content: message[1].content
                    });
                }

                // sort messages by timestamp, putting the most recent last
                // since additional messages can be more efficiently appended to the end of the list
                // rather than the beginning
                messageList.sort((m1, m2) => m1.timestamp - m2.timestamp);

                channelList.push({
                    id: channel[1].id,
                    name: channel[1].name,
                    position: channel[1].rawPosition,
                    messages: messageList
                });
            }
        }

        // sort channels by their position, as returned by the Discord API
        channelList.sort((c1, c2) => c1.position - c2.position);

        servers[guild[1].id] = {
            id: guild[1].id,
            name: guild[1].name,
            textchannels: channelList
        };
    }

    console.log('Initialization Complete!')

    initialized = true;
});

client.on('messageCreate', async (message) => {
    // if messages arrive during initialization, they might not be returned by the API
    if(!initialized) return;

    if(message.guildId in servers) {
        console.log(`Got message in ${servers[message.guildId].name}: ${message}`)

        let channelFound = false;
        for(const channel of servers[message.guildId].textchannels) {
            if(channel.id === message.channelId) {
                channel.messages.push({
                    id: message.id,
                    nickname: message.member === null ? message.author.username : message.member.displayName,
                    timestamp: message.createdTimestamp,
                    content: message.content
                });
                channelFound = true;
                break;
            }
        }

        if(!channelFound) {
            console.warn(`Message came from unknown channel ${message.channelId}`);
        }
    } else {
        console.warn(`Message "${message}" came from unknown server ${message.guildId}`);
    }


    lastMessage = message.content;
});

app.get('/', (req, res) => {
    console.log(`Got request to /: ${JSON.stringify(req.headers)}`);
    res.json(servers);
});

app.get('/servers', (req, res) => {
    console.log(`Got request to /servers: ${JSON.stringify(req.headers)}`);
    
    let retVal = '';
    for(const server of servers) {
        retVal += 's' + server.name + '\n';
        for(const channel of server.textchannels) {
            retVal += channel.id + ',' + channel.name + '\n';
        }
    }
    res.send(retVal);
});

app.get('/get/:id/:scroll', (req, res) => {
    console.log(`Got request to /get: ${JSON.stringify(req.headers)}`);
    console.log(`Parameters: ${JSON.stringify(req.params)}`);

    let scroll = parseInt(req.params.scroll);
    if(scroll === NaN) scroll = 0;
    
    
    for(const [id, server] of Object.entries(servers)) {
        for(const channel of server.textchannels) {
            if(req.params.id === channel.id) {
                res.send('DISCORD_START\n' + stringf.displayMessages(channel.messages, scroll));
                return;
            }
        }
    }

    res.send('Error: Channel not found');
    return;
});
  
client.login(secret.token);

app.listen(port, () => {
    console.log(`Discord web server listening on port ${port}`);
});

