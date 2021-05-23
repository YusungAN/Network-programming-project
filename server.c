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

void nonblock(int sockfd);

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
                    }while((readn = read(sockfd, buf, sizeof(buf) - 1)) > 0);
                
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
