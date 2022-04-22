const secret = require('./secret').secret;

const express = require('express');
const app = express();
const port = 3000;

const { Client, Intents } = require('discord.js');
const client = new Client({ intents: [Intents.FLAGS.GUILDS, Intents.FLAGS.GUILD_MESSAGES] });

let lastMessage = '';

client.on('ready', () => {
	console.log(`Logged in as ${client.user.tag}!`);
});

client.on('messageCreate', async (message) => {
    console.log(`message is created -> ${message}`);
    lastMessage = message.content;
});

app.get('/', (req, res) => {
    console.log(`Got request: ${req.headers}`);
    res.send(lastMessage);
});
  
client.login(secret.token);

app.listen(port, () => {
    console.log(`Discord web server listening on port ${port}`);
});

