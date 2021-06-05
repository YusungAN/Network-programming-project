#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
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
	while (1)
	{
		memset(message, 0, BUF_SIZE);
		fputs("Input message(Q to quit): ", stdout);
		//fgets(message, BUF_SIZE, stdin);

		scanf("%d %s", &dest, message);
		if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
			break;

		sprintf(temp, "0001%04ld%04d%s", strlen(message) + 4, dest, message);
		write(sock, temp, strlen(temp));
		str_len = read(sock, message, BUF_SIZE);
		printf("Message from server: %s", message);
	}

	close(sock);
	return 0;
}
