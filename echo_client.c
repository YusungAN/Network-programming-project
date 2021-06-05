#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "protocol.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	int sock;
	int dest;
	char message[BUF_SIZE];
	char temp[BUF_SIZE];
	int str_len;
	struct sockaddr_in serv_adr;

	fd_set readfds, allfds;
	int fd_num;

	if (argc != 3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	char nickname[30];

	printf("Input Nickname: ");
	scanf("%s", nickname);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("socket() error");
		exit(0);
	}

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
	{
		printf("connect() error!");
		exit(0);
	}
	else
		printf("Connected...........\n");

	sprintf(message, "0000%04ld%s", strlen(nickname), nickname);
	write(sock, message, strlen(message));

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	FD_SET(sock, &readfds);

	while (1)
	{
		allfds = readfds;
		printf("Input message(Q to quit):\n");
		fd_num = select(4, &allfds, (fd_set *)0, (fd_set *)0, NULL);

		if (fd_num < 0)
		{
			printf("select() error");
			exit(0);
		}

		if (FD_ISSET(STDIN_FILENO, &allfds))
		{
			memset(message, 0, BUF_SIZE);
			//fgets(message, BUF_SIZE, stdin);

			if (read(0, message, 1) < 0)
				continue;
			dest = atoi(message);
			str_len = read(0, message, BUF_SIZE);
			if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
				break;

			sprintf(temp, "0001%04ld%04d%s", strlen(message) + 4, dest, message);
			write(sock, temp, strlen(temp));
		}

		if (FD_ISSET(sock, &allfds))
		{
			if ((str_len = read(sock, message, BUF_SIZE)) > 0)
			{
				message[str_len] = 0;
				printf("\nMessage from server: %s\n", message);
			}
			else
			{
				printf("Connection reset\n");
				close(sock);
				exit(1);
			}
		}
	}

	close(sock);
	return 0;
}
