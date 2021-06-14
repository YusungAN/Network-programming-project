#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protocol.h"

#define MAXLINE 1024
#define PORTNUM 9100
#define SOCK_SETSIZE 1021
#define INTERVAL 10
#define MULTICAST_INTERVAL 2000

#define MAX_CLIENT 30
#define MAX_NICKNAME_LENGTH 30

void nonblock(int sockfd);
void printonlineusers(userdata *arr);
void itoa(int i, char *st);
int fetchUser(userdata *data, char *target);
void broadcastUserMsg(userdata *data);

int main(int argc, char **argv)
{
    int listen_fd, client_fd;
    socklen_t addrlen;
    int fd_num;
    int maxfd = 0;
    int sockfd;
    int readn;
    int i = 0;
    int on;
    int timer = 0;

    int client = 0;

    UserLogin loginArr[7];
    strcpy(loginArr[0].nickname, "yusung");
    strcpy(loginArr[0].password, "0510");
    strcpy(loginArr[1].nickname, "yubin");
    strcpy(loginArr[1].password, "0119");
    strcpy(loginArr[2].nickname, "hojin");
    strcpy(loginArr[2].password, "0906");
    strcpy(loginArr[3].nickname, "taehwan");
    strcpy(loginArr[3].password, "1006");
    strcpy(loginArr[4].nickname, "seongjip");
    strcpy(loginArr[4].password, "0510");
    strcpy(loginArr[5].nickname, "youngmin");
    strcpy(loginArr[5].password, "0318");
    strcpy(loginArr[6].nickname, "soonjae");
    strcpy(loginArr[6].password, "0322");

    userdata u_data[MAX_CLIENT];
    memset(u_data, 0, sizeof(u_data));
    int sock_u_map[MAX_CLIENT] = {
        0,
    };
    messagedata m_data;

    char aBuffer[5];
    char buf[MAXLINE];
    fd_set readfds, allfds;
    request req;

    struct sockaddr_in server_addr, client_addr;

    //multicast server information
    int multicast_sock;
    int ttl = 2;
    struct timeval multicast_tv;
    int multicast_state;
    char multicast_ip[20];
    strcpy(multicast_ip, "239.0.110.1");
    int multicast_port = 5100;

    multicast_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (multicast_sock == -1)
        printf("socket() error");

    struct sockaddr_in multicast_serv_addr;

    memset(&multicast_serv_addr, 0, sizeof(multicast_serv_addr));
    multicast_serv_addr.sin_family = AF_INET;
    multicast_serv_addr.sin_addr.s_addr = inet_addr(multicast_ip);
    multicast_serv_addr.sin_port = htons(multicast_port);

    setsockopt(multicast_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl, sizeof(ttl));
    //---------

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket error");
        return 1;
    }
    memset((void *)&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORTNUM);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind error");
        return 1;
    }
    if (listen(listen_fd, 5) == -1)
    {
        perror("listen error");
        return 1;
    }

    on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);

    maxfd = listen_fd;
    //make CM info message
    send_CM_info send_msg;
    send_msg.opcode = 1000;
    send_msg.port = 9100;
    strcpy(send_msg.ip, "192.168.100.92");

    char CM_info_message[1024];

    char opcode[4] = {
        0,
    };
    char port[4] = {
        0,
    };
    int off = 0;

    memset(&CM_info_message, 0, sizeof(CM_info_message));
    // opcode
    itoa(send_msg.opcode, opcode);
    memcpy(&CM_info_message[off], &opcode, sizeof(opcode));
    off = sizeof(opcode);

    // port
    itoa(send_msg.port, port);
    memcpy(&CM_info_message[off], &port, sizeof(port));
    off += sizeof(port);

    // ip
    memcpy(&CM_info_message[off], &send_msg.ip, sizeof(send_msg.ip));
    off += sizeof(send_msg.ip);

    multicast_tv.tv_sec = 2;
    multicast_tv.tv_usec = 0;

    while (1)
    {
        allfds = readfds;
        fd_num = select(maxfd + 1, &allfds, (fd_set *)0,
                        (fd_set *)0, &multicast_tv);

        if (fd_num == 0)
        {
            printf("\x1b[H\x1b[Jsend server info\n");
            printf("%s\n", &CM_info_message[0]);
            sendto(multicast_sock, CM_info_message, strlen(CM_info_message), 0, (struct sockaddr *)&multicast_serv_addr, sizeof(multicast_serv_addr));

            printonlineusers(u_data);
            multicast_tv.tv_sec = 2;
            multicast_tv.tv_usec = 0;
        }

        if (FD_ISSET(listen_fd, &allfds)) // accept()
        {
            addrlen = sizeof(client_addr);
            client_fd = accept(listen_fd,
                               (struct sockaddr *)&client_addr, &addrlen);
            //nonblock(client_fd);
            FD_SET(client_fd, &readfds);

            if (client_fd > maxfd)
                maxfd = client_fd;
            printf("Accept OK\n");
            continue;
        }

        for (i = 0; i <= maxfd; i++) // socket byte read
        {
            sockfd = i;
            if (FD_ISSET(sockfd, &allfds))
            {
                memset(buf, 0, sizeof(buf));
                if ((readn = read(sockfd, buf, sizeof(buf) - 1)) > 0)
                {
                    printf("[RECV] %s\n", buf);
                    memset(&req, 0, sizeof(req));
                    memset(aBuffer, 0, sizeof(aBuffer));
                    memcpy(aBuffer, buf, 4);
                    req.opcode = atoi(aBuffer);
                    memcpy(aBuffer, &buf[4], 4);
                    req.length = atoi(aBuffer);
                    
                    memcpy(req.data, &buf[8], req.length); //req.length);

                    switch (req.opcode)
                    {

                    case 0x0: // Initial Connecting Information
                        /*
                            Data - 30Byte
                            Opcode(4) Length(4) Data( Nick(30) ) EOF
                        */
                       {
                        int login_success = 0;

                        int nick_length = 0, pw_length = 0;
                        memset(aBuffer, 0, sizeof(aBuffer));
                        memcpy(aBuffer, &buf[8], 4);
                        nick_length = atoi(aBuffer);
                        memset(aBuffer, 0, sizeof(aBuffer));
                        memcpy(aBuffer, &buf[12+nick_length], 4);
                        pw_length = atoi(aBuffer); 

                        char nickname[30];
                        memset(nickname, 0, sizeof(nickname));
                        memcpy(nickname, &buf[12], nick_length); 

                        char password[30];
                        memset(password, 0, sizeof(password));
                        memcpy(password, &buf[16+nick_length], nick_length);
                        printf("%s %s\n", nickname, password);
                        for (int i = 0; i < 7; i++) {
                            if (!strcmp(nickname, loginArr[i].nickname) && !strcmp(password, loginArr[i].password)) {
                                login_success = 1;
                                break;
                            }
                        }

                        if (nick_length > 30)
                        {
                            perror("Nickname cannot be longer than 30 letters\n");
                        }
                        if (pw_length > 30)
                        {
                            perror("Password cannot be longer than 30 letters\n");
                        }
                        if (login_success == 0)
                        {
                            perror("Nickname and pw does not match");
                        }
                        else if (login_success)
                        {
                            if ((on = fetchUser(u_data, nickname)) > -1)
                            {
                                sock_u_map[sockfd] = on;
                                u_data[sock_u_map[sockfd]].sockfd = sockfd;
                                u_data[sock_u_map[sockfd]].connected = 2;
                            }
                            else
                            {
                                sock_u_map[sockfd] = client;
                                memset(&u_data[client], 0, sizeof(userdata));
                                memcpy(&u_data[client].nick, nickname, MAX_NICKNAME_LENGTH);
                                u_data[client].connected = 2;
                                u_data[client++].sockfd = sockfd;
                            }
                            broadcastUserMsg(u_data);
                        }

                        printf("[%d] %s Connected.\n", u_data[sock_u_map[sockfd]].sockfd, u_data[sock_u_map[sockfd]].nick);
                        //printonlineusers(u_data);
                       }
                        break;

                    case 0x1: // Message Send Request
                              /*
                            Data - ~ 1012 byte
                            Opcode(4) Length(4) Data( Desc(4) Data(1012) )
                        */
                        memset(&m_data, 0, sizeof(m_data));
                        memset(aBuffer, 0, sizeof(aBuffer));
                        memcpy(aBuffer, &req.data, 4);
                        m_data.dest = atoi(aBuffer);
                        if (m_data.dest < 5)
                            continue;
                        memcpy(m_data.message, &req.data[4], req.length - 4);

                        memset(buf, 0, sizeof(buf));
                        sprintf(buf, "1000%04ld%04d%s", strlen(m_data.message) + 4, sockfd, m_data.message);
                        write(m_data.dest, buf, strlen(buf));
                        break;

                    default:
                        break;
                    }

                    buf[readn] = 0;
                    printf("[%d] = %s\n", sockfd, req.data);
                    //write(sockfd, buf, strlen(buf));
                }
                else
                {
                    printf("close\n");
                    u_data[sock_u_map[sockfd]].connected = 1;
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                    broadcastUserMsg(u_data);
                }

                if (--fd_num <= 0)
                    break;
            }
        }
    }
}

void nonblock(int sockfd) // Making Socket non-blocking socket
{
    int opts;
    opts = fcntl(sockfd, F_GETFL);
    if (opts < 0)
    {
        printf("fctntl(F_GETFL) error\n");
        exit(1);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        printf("fcntl(F_SETFL) error\n");
        exit(1);
    }
}

void printonlineusers(userdata *arr)
{
    printf("\nCurrently Online:\n----------------------\n");
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (arr[i].connected == 2)
        {
            printf("[%d] %s \x1b[32m● Online\x1b[0m\n", arr[i].sockfd, arr[i].nick);
        }
        else if (arr[i].connected == 1)
        {
            printf("[-] %s \x1b[31m● Offline\x1b[0m\n", arr[i].nick);
        }
    }
    printf("----------------------\n");
}

void itoa(int i, char *st)
{
    sprintf(st, "%d", i);
}

int fetchUser(userdata *data, char *target)
{
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (!strcmp(data[i].nick, target))
            return i;
    }
    return -1;
}

void broadcastUserMsg(userdata *data)
{
    char buf[35 * MAX_CLIENT + 1] = {
        0,
    };
    char message[1024] = {
        0,
    };
    int offset = 0;
    int cnt = 0;
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (data[i].connected >= 1)
        {

            memcpy(&buf[offset], data[i].nick, MAX_NICKNAME_LENGTH);
            offset += MAX_NICKNAME_LENGTH;
            sprintf(&buf[offset], "%04d%d", data[i].sockfd, data[i].connected);
            offset += 5;
            cnt++;
        }
    }

    sprintf(message, "1001%04d%04d", offset + 5, cnt);
    memcpy(&message[12], buf, offset);

    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (data[i].connected == 2)
            write(data[i].sockfd, message, sizeof(message));
    }

    fwrite(message, sizeof(message), 1, stdin);
}