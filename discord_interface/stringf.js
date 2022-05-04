// this file contains string functions that are useful for preparing messages to be displayed on the FPGA

// configurable constants

// this should also be enforced by the FPGA so the user is able to scroll back down
const MAX_SCROLL = 255;

// screen region size constants
const SIDEBAR_WIDTH = 120;

const MESSAGE_REGION_WIDTH = 480;
const MESSAGE_REGION_HEIGHT = 400;

// constants from the font data
const types = {
    UNAME: 0,
    MESSAGE: 1,
    CHANNEL: 2,
    SERVER: 3,
    TITLE: 4
}

// username font
const UNAME_TEXT_HEIGHT = 14;
const uname_widths = [8, 5, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 5, 7, 7, 4, 4, 7, 5, 10, 7, 7, 7, 7, 5, 6, 5, 7, 7, 10,
    8, 7, 7, 9, 9, 8, 9, 8, 8, 9, 9, 4, 8, 9, 8, 10, 9, 9, 9, 9, 9, 8, 8, 9, 9, 12, 9, 9, 8, 4, 6, 9, 8, 10, 9, 4, 5, 5, 7,
    8, 4, 6, 4, 7, 4, 4, 6, 7, 6, 7, 10, 5, 7, 5, 8, 8, 3, 5, 4, 5, 7, 2];

// message font
const MESSAGE_TEXT_MARGIN = 1;
const MESSAGE_TEXT_HEIGHT = 14;
const message_widths = [8, 4, 8, 8, 8, 8, 8, 7, 8, 8, 7, 7, 7, 7, 7, 5, 7, 7, 3, 3, 7, 4, 10, 7, 7, 7, 7, 5, 6, 5, 7, 7, 10,
    7, 7, 7, 9, 9, 8, 8, 8, 8, 9, 9, 3, 8, 9, 7, 10, 9, 9, 8, 9, 9, 8, 8, 9, 9, 13, 8, 8, 8, 3, 5, 8, 8, 10, 9, 3, 4, 4, 6, 8,
    3, 5, 3, 6, 4, 4, 6, 7, 6, 7, 10, 4, 6, 4, 8, 8, 3, 5, 3, 5, 7, 2];

// channel font
const CHANNEL_TEXT_HEIGHT = 18;
const channel_widths = [11, 6, 10, 10, 10, 10, 11, 10, 11, 10, 8, 9, 8, 9, 9, 6, 9, 9, 4, 4, 9, 5, 13, 9, 9, 9, 9, 6, 8, 6, 9,
    9, 13, 9, 9, 8, 11, 11, 11, 11, 10, 10, 11, 11, 4, 10, 12, 9, 13, 11, 11, 11, 11, 11, 10, 10, 11, 11, 16, 11, 11, 10, 4, 6,
    11, 10, 13, 11, 4, 5, 5, 8, 10, 4, 7, 4, 8, 4, 5, 8, 9, 8, 9, 13, 5, 8, 5, 10, 10, 4, 6, 4, 6, 9, 2];

// server name font
const SERVER_TEXT_HEIGHT = 16;
const server_widths = [9, 6, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8, 8, 8, 8, 6, 8, 8, 5, 5, 9, 6, 11, 8, 8, 8, 8, 6, 7, 6, 8, 9, 12, 9,
    8, 8, 11, 10, 9, 10, 9, 9, 10, 10, 5, 9, 11, 9, 11, 10, 10, 10, 10, 11, 9, 9, 10, 11, 14, 11, 11, 10, 5, 7, 10, 9, 11, 11, 5,
    6, 5, 8, 9, 5, 6, 5, 8, 5, 5, 7, 8, 7, 8, 12, 6, 8, 5, 10, 9, 6, 6, 5, 6, 8, 2];

// title font (channel name at the top)
const TITLE_TEXT_HEIGHT = 26;
const title_widths = [15, 9, 14, 14, 14, 14, 15, 14, 15, 15, 12, 13, 12, 12, 12, 8, 12, 13, 6, 6, 14, 8, 18, 13, 12, 13, 12, 9,
    11, 9, 13, 13, 18, 14, 13, 12, 17, 16, 15, 16, 14, 14, 16, 16, 7, 14, 17, 14, 19, 16, 16, 16, 17, 17, 14, 15, 16, 17, 23, 17,
    17, 15, 7, 10, 16, 14, 18, 17, 6, 8, 8, 12, 14, 7, 10, 6, 12, 7, 7, 11, 13, 11, 12, 19, 8, 12, 8, 16, 15, 5, 9, 6, 9, 12, 2];

// unknown character placeholder (ASCII 255)
const UNKNOWN_CHAR = 'ÿ';

// ASCII decoding array
const decode_ascii = [
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 
    73, 74, 75, 76, 0, 1, 2, 3, 4, 5, 6, 7, 8, 
    9, 77, 78, 79, 80, 81, 82, 83, 36, 37, 38, 
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 
    50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 
    61, 84, 85, 86, 87, 88, 89, 10, 11, 12, 13, 
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
    25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 
    90, 91, 92, 93, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 94, 
    94
];

// takes Javascript string representation of character and type
// returns [ascii character, width]
// handles bounds checking and returns unknown character where appropriate
function getCharWidth(s, type) {
    if (type > 4 || type < 0) {
        console.error(`getWidth got invalid type: ${type}`);
        return 0;
    }

    // special case for space character
    if(s === ' ') {
        if(type === types.SERVER) {
            return 4;

        } else {
            return 3;
        }
    }

    const code = s.charCodeAt(0);

    // unknown character
    if(code > 127 || code < 0) {
        if(type === types.UNAME) {
            return [UNKNOWN_CHAR, Math.floor(UNAME_TEXT_HEIGHT/2)];
        } else if (type === types.CHANNEL) {
            return [UNKNOWN_CHAR, Math.floor(CHANNEL_TEXT_HEIGHT/2)];
        } else if (type === types.MESSAGE) {
            return [UNKNOWN_CHAR, Math.floor(MESSAGE_TEXT_HEIGHT/2)];
        } else if (type === types.SERVER) {
            return [UNKNOWN_CHAR, Math.floor(SERVER_TEXT_HEIGHT/2)];
        } else if (type === types.TITLE) {
            return [UNKNOWN_CHAR, Math.floor(TITLE_TEXT_HEIGHT/2)];
        }
    } else {
        if(type === types.UNAME) {
            return [s.charAt(0), uname_widths[decode_ascii[code]]];
        } else if (type === types.CHANNEL) {
            return [s.charAt(0), channel_widths[decode_ascii[code]]];
        } else if (type === types.MESSAGE) {
            return [s.charAt(0), message_widths[decode_ascii[code]]];
        } else if (type === types.SERVER) {
            return [s.charAt(0), server_widths[decode_ascii[code]]];
        } else if (type === types.TITLE) {
            return [s.charAt(0), title_widths[decode_ascii[code]]];
        }
    }
}

// clean up server name strings so they fit on the side bar
exports.cleanServerName = function (name) {
    let cleanName = '';

    let chars = name.split('');
    let totalLength = 0;
    
    // iterate through character indexes
    for(const i in chars) {
        [char, len] = getCharWidth(chars[i], types.SERVER);

        // if the character fits, add it, otherwise break out of the loop
        if(totalLength + len <= SIDEBAR_WIDTH) {
            cleanName += char;
            totalLength += len;
        } else {
            break;
        }
    }

    return cleanName;
}

// clean up channel name strings so they fit on the top bar
exports.cleanTitleName = function (name) {
    let cleanName = '';

    // prepend '#' to channel names
    let chars = ('#' + name).split('');
    let totalLength = 0;
    
    // iterate through character indexes
    for(const i in chars) {
        [char, len] = getCharWidth(chars[i], types.TITLE);

        // if the character fits, add it, otherwise break out of the loop
        if(totalLength + len <= MESSAGE_REGION_WIDTH) {
            cleanName += char;
            totalLength += len;
        } else {
            break;
        }
    }

    //console.log(totalLength);

    return cleanName;
}


// clean up channel name strings so they fit on the side bar
exports.cleanChannelName = function (name) {
    let cleanName = '';

    // prepend '#' to channel names
    let chars = ('#' + name).split('');
    let totalLength = 0;
    
    // iterate through character indexes
    for(const i in chars) {
        [char, len] = getCharWidth(chars[i], types.CHANNEL);

        // if the character fits, add it, otherwise break out of the loop
        if(totalLength + len <= SIDEBAR_WIDTH) {
            cleanName += char;
            totalLength += len;
        } else {
            break;
        }
    }

    //console.log(totalLength);

    return cleanName;
}

// clean up user name strings so they fit in the messages area
cleanUsername = function (name) {
    // prepend 'u' to the username
    let cleanName = 'u';

    let chars = name.split('');
    let totalLength = 0;
    
    // iterate through character indexes
    for(const i in chars) {
        [char, len] = getCharWidth(chars[i], types.UNAME);

        // if the character fits, add it, otherwise break out of the loop
        if(totalLength + len <= MESSAGE_REGION_WIDTH) {
            cleanName += char;
            totalLength += len;
        } else {
            break;
        }
    }

    //console.log(totalLength);
    return cleanName;
}

// clean up messages by inserting newlines where appropriate and prepending 'm' to each line
cleanMessage = function (message) {
    let cleanMessage = 'm';

    // prepend '#' to channel names
    let chars = message.split('');
    let totalLength = 0;
    
    // iterate through character indexes
    for(const i in chars) {
        [char, len] = getCharWidth(chars[i], types.MESSAGE);

        // if on a new line, reset the length and add an m
        if(char === '\n') {
            cleanMessage += char + 'm';
            totalLength = 0;
        }
        // if the character fits, add it, otherwise start a new line
        else if(totalLength + len <= MESSAGE_REGION_WIDTH) {
            cleanMessage += char;
            totalLength += len;
        
        // if the character is not a new line and does not fit, add a newline, m, and the character
        } else {
            cleanMessage += '\nm' + char;
            totalLength = len;
        }
    }
    return cleanMessage;
}

// converts a messages list and scroll value to a string that can easily be rendered on the screen
// this function takes into account the FPGA's limited character set and screen space

// the messages should return with most recent at the top and oldest at the bottom
// since the FPGA renders bottom to top
exports.displayMessages = function(messages, scroll) {
    //console.log(JSON.stringify(messages));

    // max supported scroll
    if(scroll > MAX_SCROLL) scroll = MAX_SCROLL;

    // holds all the cleaned usernames + messages
    let cleanMessages = []

    // step 1: convert the messages into a list of lines

    // iterating through messages oldest to newest
    for(const message of messages) {
        // get username and message lines
        const user = cleanUsername(message.nickname);
        const messageClean = cleanMessage(message.content);

        cleanMessages.push(user);
        // reverse message order
        cleanMessages.push(messageClean.split('\n').reverse().join('\n'));
    }

    // flip the messages so they are newest to oldest
    cleanMessages.reverse();

    // convert into a list of lines that could be drawn by the FPGA
    let lines = cleanMessages.join('\n').split('\n');

    // message at the end of scrolling
    lines.push('u[End of message history]');

    // step 2: apply scroll to compute the final return value

    let retVal = '';
    for(let i = scroll; i < scroll+Math.floor((MESSAGE_REGION_HEIGHT/(MESSAGE_TEXT_HEIGHT+2*MESSAGE_TEXT_MARGIN))); i += 1) {

        // if scrolled beyond text, just send a blank line
        if(i >= lines.length) {
            retVal += 'm\n';
        } else {
            retVal += lines[i] + '\n';
        }
    }

    return retVal;
}

function test() {
    // console.log(decode_ascii.length);
    // console.log('ÿ'.charCodeAt(0));
    // console.log(decode_ascii['?'.charCodeAt(0)])
    // console.log(message_widths[decode_ascii['?'.charCodeAt(0)]])
    // console.log(uname_widths[decode_ascii['?'.charCodeAt(0)]])
    // console.log(getCharWidth('i', types.CHANNEL))

    console.log(exports.cleanServerName('Øemoji😊movieisthebestmovieofalltime'));
    console.log(exports.cleanTitleName('nnewlinesqweoinjpwiofdjopasidfjopweirjwpeiorjwopierjwopierjw😊😊😊😊jrweopirjpwoeirjweioprweiopjropweijropweijrpowiejropweirjopij'));

    console.log(exports.cleanChannelName('aiØdjqiw😊qwoeijiaudfhnasieuyhnq8owuerynwqeuiryneqwiouyrnoiweuyuiodahsfuiodshfwuiorehoweuirh'));
    console.log(cleanMessage('message\nwithsome\nnewlinesqweoirjpwiofdjopasidfjopweirjwpeiorjwopierjwopierjw😊😊😊😊jrweopirjpwoeirjweioprweiopjropweijropweijrpowiejropweirjopij'));
    console.log(cleanMessage('message\nwithsome\nnewlinesqweoirjpwiofdjopasidfjopweirjwpeiorjwopierjwopierjw😊😊😊😊jrweopirjpwoeirjweiopr\nweiopjropweijropweijrpowiejropweirjopij'));
    console.log(cleanUsername('Captain_Suskoooooooooooo😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊😊end'));
    console.log(cleanUsername('[End of message history]'))
    console.log(['a','b'].concat(['c','d']));

    // number of lines
    console.log(Math.floor((MESSAGE_REGION_HEIGHT/(MESSAGE_TEXT_HEIGHT+2*MESSAGE_TEXT_MARGIN))))
}

// run tests 
// test();