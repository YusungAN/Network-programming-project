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

void nonblock(int sockfd);
void itoa(int i, char *st);

int main(int argc, char **argv)
{
    int listen_fd, client_fd;
    socklen_t addrlen;
    int fd_num;
    int maxfd = 0;
    int sockfd;
    int readn;
    int i = 0;
    char buf[MAXLINE];
    fd_set readfds, allfds;

    struct sockaddr_in server_addr, client_addr;

    //multicast server information
    int multicast_sock;
    int ttl = 2;
    struct timeval multicast_tv;
    int multicast_state;
    char multicast_ip[20];
    strcpy(multicast_ip, "239.0.110.1");
	int multicast_port = 5100;

    multicast_sock=socket(PF_INET, SOCK_DGRAM, 0);
	if(multicast_sock == -1)
		printf("socket() error");

    struct sockaddr_in multicast_serv_addr;

    memset(&multicast_serv_addr, 0, sizeof(multicast_serv_addr));
	multicast_serv_addr.sin_family=AF_INET;
	multicast_serv_addr.sin_addr.s_addr=inet_addr(multicast_ip);
	multicast_serv_addr.sin_port=htons(multicast_port);

    setsockopt(multicast_sock, IPPROTO_IP, IP_MULTICAST_TTL, (void*)&ttl, sizeof(ttl));
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

    FD_ZERO(&readfds);
    FD_SET(listen_fd, &readfds);

    maxfd = listen_fd;

    //make CM info message
    send_CM_info send_msg;
    send_msg.opcode = 1000;
    send_msg.port = 5100;
    strcpy(send_msg.ip, "192.168.100.92");

    char CM_info_message[1024];

    char opcode[4] = {0,};
    char port[4] = {0,};
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


    while (1)
    {
        //multicast server info
        multicast_tv.tv_sec = 2;
        multicast_tv.tv_usec = 0;

        allfds = readfds;
        fd_num = select(maxfd + 1, &allfds, (fd_set *)0,
                        (fd_set *)0, &multicast_tv);

        if (fd_num == 0) {
            printf("send server info\n");
            printf("%s\n", &CM_info_message[0]);
            sendto(multicast_sock, CM_info_message, strlen(CM_info_message), 0, (struct sockaddr*)&multicast_serv_addr, sizeof(multicast_serv_addr));
        }

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
                if ((readn = read(sockfd, buf, MAXLINE - 1)) == 0)
                {
                    printf("close\n");
                    close(sockfd);
                    FD_CLR(sockfd, &readfds);
                }
                else
                {

                    do
                    {
                        buf[readn] = 0;
			            printf("[%d] = %s", sockfd, buf); 
                        write(sockfd, buf, strlen(buf));
                    } while((readn = read(sockfd, buf, sizeof(buf) - 1)) > 0);
                
		    if(readn == -1){
			if(errno != EAGAIN)
			{
				close(sockfd);
				FD_CLR(sockfd, &readfds);
			}
		    }
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
	if(opts < 0)
	{
		printf("fctntl(F_GETFL) error\n");
		exit(1);
	}
	opts = (opts | O_NONBLOCK);
	if(fcntl(sockfd, F_SETFL, opts) < 0)
	{
		printf("fcntl(F_SETFL) error\n");
		exit(1);
	}

}

void itoa(int i, char *st) {
        sprintf(st, "%d", i);
}