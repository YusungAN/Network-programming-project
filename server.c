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

#define MAXLINE 1024
#define PORTNUM 9100
#define SOCK_SETSIZE 1021

#define MAX_CLIENT 30
#define MAX_NICKNAME_LENGTH 30

typedef struct
{
    int opcode;
    int length;
    char data[1024];
} request;

typedef struct
{
    char nick[30];
    int sockfd;
    int connected;
} userdata;

void nonblock(int sockfd);
void printonlineusers(userdata *arr);

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

    userdata u_data[MAX_CLIENT];

    char aBuffer[5];
    char buf[MAXLINE];
    fd_set readfds, allfds;
    request req;

    struct sockaddr_in server_addr, client_addr;

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
    while (1)
    {
        allfds = readfds;
        fd_num = select(maxfd + 1, &allfds, (fd_set *)0,
                        (fd_set *)0, NULL);

        if (FD_ISSET(listen_fd, &allfds)) // accept()
        {
            addrlen = sizeof(client_addr);
            client_fd = accept(listen_fd,
                               (struct sockaddr *)&client_addr, &addrlen);
            nonblock(client_fd);
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

                if ((readn = read(sockfd, buf, sizeof(buf) - 1)) > 0)
                {
                    printf("%s\n", &buf[8]);
                    memset(&req, 0, sizeof(req));
                    memset(aBuffer, 0, sizeof(aBuffer));
                    memcpy(aBuffer, buf, 4);
                    req.opcode = atoi(aBuffer);
                    memcpy(aBuffer, &buf[4], 4);
                    req.length = atoi(aBuffer);
                    printf("%d\n", req.length);
                    memcpy(req.data, &buf[8], req.length); //req.length);

                    printf("%d %d %s\n", req.opcode, req.length, req.data);

                    switch (req.opcode)
                    {

                    case 0x0: // Initial Connecting Information
                        /*
                            Data - 30Byte
                            Opcode(4) Length(4) Data( Nick(30) ) EOF
                        */

                        if (req.length > 30)
                        {
                            perror("Nickname cannot be longer than 30 letters");
                        }
                        else
                        {
                            memset(&u_data[sockfd], 0, sizeof(userdata));
                            memcpy(&u_data[sockfd].nick, req.data, MAX_NICKNAME_LENGTH);
                            u_data[sockfd].connected = 1;
                        }

                        printonlineusers(u_data);

                        break;

                    case 0x1: // Message Send Request
                        /*
                            Data - ~ 1028 byte
                            Opcode(4) Length(4) Data( Desc(4) Data(1024) )
                        */
                        break;

                    default:
                        break;
                    }

                    buf[readn] = 0;
                    printf("[%d] = %s", sockfd, req.data);
                    write(sockfd, buf, strlen(buf));
                }
                else
                {
                    printf("close\n");
                    u_data[sockfd].connected = 0;
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                    printonlineusers(u_data);
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
    printf("\nCurrently Online:\n");
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        if (arr[i].connected == 1)
        {
            printf("[%d] %s\n", arr[i].sockfd, arr[i].nick);
        }
    }
}