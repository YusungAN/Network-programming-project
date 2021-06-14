#include <string.h>
#include <stdlib.h>
#include <string.h>

// Use this function when your client have to parse Chatting Manger Server information4
// *data: received Server information / *opcode, *port, *ip: Variable to save opcode/port/ip information
void parseCMInfo(char *data, int *opcode, int *port, char *ip)
{
    char opcodeTemp[5] = {
        0,
    };
    char portTemp[5] = {
        0,
    };
    strncpy(opcodeTemp, data, 4);
    strncpy(portTemp, data + 4, 4);
    strcpy(ip, &data[8]);
    *opcode = atoi(opcodeTemp);
    *port = atoi(portTemp);
}

// Generating Message Request Protocol String - 0001LENGDESTMessage...
// dest: Recipient Client sockfd / *str: message / *buf: target buffer
void msgRequest(int dest, char *str, char *buf)
{
    sprintf(buf, "0001%04ld%04d%s", strlen(str) + 4, dest, str);
}

// Generating Initial Handshake Protocol String - 0000LENGDESTNickname...
// *str: nickname  / *buf: target buffer
void initialHandshakeRequest(char *str, char *str2, char *buf)
{
    char pw_length[5];
    sprintf(buf, "0000%04ld%04ld", strlen(str)+strlen(str2), strlen(str));
    memcpy(&buf[strlen(buf)], str, 30);
    sprintf(pw_length, "%04ld", strlen(str2));
    memcpy(&buf[strlen(buf)], pw_length, strlen(pw_length));
    memcpy(&buf[strlen(buf)], str2, sizeof(str2));
}

void parseChatRelay(char *protocol, int *sender, char *message)
{
    char aBuffer[5] = {
        0,
    };
    memcpy(aBuffer, &protocol[8], 4);
    *sender = atoi(aBuffer);
    memcpy(aBuffer, &protocol[4], 4);
    memcpy(message, &protocol[12], atoi(aBuffer) - 4);
}

void parseOpcode(char *opcode, int *target)
{
    char *p = opcode;
    int r = 0;
    while (p && *p)
    {
        r <<= 1;
        r += ((*p++) & 0x01);
    }
    *target = r;
}

void parseUserList(char *buf, userdata *list)
{
    int cnt;
    char aBuffer[5] = {
        0,
    };
    memcpy(aBuffer, buf, 4);
    cnt = atoi(aBuffer);
    int offset = 4;
    for (int i = 0; i < cnt; i++)
    {
        memcpy(list[i].nick, &buf[offset], 30);
        memcpy(aBuffer, &buf[offset + 30], 4);
        list[i].sockfd = atoi(aBuffer);
        switch (buf[offset + 34])
        {
        case '1':
            list[i].connected = 1;
            break;
        case '2':
            list[i].connected = 2;
            break;

        default:
            list[i].connected = 0;
            break;
        }

        offset += 35;
    }
}