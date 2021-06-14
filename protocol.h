typedef struct
{
    int opcode;  // 0~3, 1000
    int port;    // 4~7
    char ip[20]; // 7~
} send_CM_info;

typedef struct
{
    int opcode;      // 0~3 opcode, (int)
    int length;      // 3~7 data length, (int)
    char data[1016]; // 8~ data (char[1016])
} request;

typedef request respond;

typedef struct
{
    char nick[30];
    int sockfd;
    int connected;
} userdata; // userdata

typedef struct {
    char nickname[30];
    char password[30];
} UserLogin;

typedef struct
{
    int dest;
    char message[1012];
} messagedata;