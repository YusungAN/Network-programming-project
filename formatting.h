#include "formatting.c"

void parseCMInfo(char *data, int *opcode, int *port, char *ip);
void msgRequest(int dest, char *str, char *buf);
void initialHandshakeRequest(char *str, char *buf);
