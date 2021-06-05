#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "protocol.h"
#include "formatting.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[])
{
	int sock;
	int dest;
	char message[BUF_SIZE];
	char temp[BUF_SIZE];
	int str_len;
	int sender;
	struct sockaddr_in serv_adr;

	fd_set readfds, allfds;
	int fd_num;
	respond res;
	char aBuffer[5];

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

	initialHandshakeRequest(nickname, message);
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

			msgRequest(dest, message, temp);
			write(sock, temp, strlen(temp));
		}

		if (FD_ISSET(sock, &allfds))
		{
			memset(message, 0, sizeof(message));
			memset(temp, 0, sizeof(temp));
			if ((str_len = read(sock, message, BUF_SIZE)) > 0)
			{
				memset(&res, 0, sizeof(res));
				memset(aBuffer, 0, 5);
				memcpy(aBuffer, message, 4);
				parseOpcode(aBuffer, &res.opcode);
				memcpy(aBuffer, &message[4], 4);
				res.length = atoi(aBuffer);
				memcpy(res.data, &message[8], res.length);

				switch (res.opcode)
				{

				case 0x8:
					// Server Message Relay
					/*
                            Data - ~ 1012 byte
                            Opcode(4) Length(4) Data( Sender(4) Data(1012) )
                    */
					parseChatRelay(message, &sender, temp);
					printf("\nMessage from [ %d ]: %s\n", sender, temp);
					break;
				case 0xF:
					// Server Error ACK
					/*
							Errno - 4 byte Message - ~ 1012 bytes
							Opcode(4) Length(4) Data( Errno(4) Data(1012) )
					*/

					// TODO

					break;
				default:
					break;
				}
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
