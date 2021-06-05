#include <string.h>
#include <stdlib.h>

// Use this function when your client have to parse Chatting Manger Server information4
// *data: received Server information / *opcode, *port, *ip: Variable to save opcode/port/ip information
void parseCMInfo(char *data, int *opcode, int *port, char *ip) {
    char opcodeTemp[5] = {0,};
    char portTemp[5] = {0,};
    strncpy(opcodeTemp, data, 4);
    strncpy(portTemp, data+4, 4);
    strcpy(ip, &data[8]);
    *opcode = atoi(opcodeTemp);
    *port = atoi(portTemp);
}