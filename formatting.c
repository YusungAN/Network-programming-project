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
void initialHandshakeRequest(char *str, char *buf)
{
    sprintf(buf, "0000%04ld%s", strlen(str), str);
}
