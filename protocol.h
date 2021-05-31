typedef struct {
    int opcode; // 0~3, 1000
    int port; // 4~7
    char ip[20]; // 7~
} send_CM_info;
