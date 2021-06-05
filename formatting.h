#include "formatting.c"

void parseCMInfo(char *data, int *opcode, int *port, char *ip);
void msgRequest(int dest, char *str, char *buf);
void initialHandshakeRequest(char *str, char *buf);
void parseChatRelay(char* protocol, int* sender, char* message);
void parseOpcode(char *opcode, int *target);