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

void printonlineusers(userdata *arr);
int fetchUser(userdata *data, int sock);
void storeList(char (*store)[1024], char *str);
void printList(char (*store)[1024]);
void printInput(int target, userdata *data);

int main()
{
	int sock;
	int dest;
	char message[BUF_SIZE];
	char temp[BUF_SIZE];
	int str_len;
	int sender;
	int errno;
	int target = -1;
	struct sockaddr_in serv_adr;
	char store[30][1024] = {
		0,
	};

	fd_set readfds, allfds;
	int fd_num;
	respond res;
	userdata u_data[30];
	memset(u_data, 0, sizeof(u_data));
	char aBuffer[5];

	int mt_sock;
	struct ip_mreq mt_join_adr;
	struct sockaddr_in mt_from_addr;
	struct sockaddr_in mt_serv_addr;
	char mt_ip[20];
	strcpy(mt_ip, "239.0.110.1");
	int mt_port = 5100;

	printf("Finding Server...\n");

	mt_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if (mt_sock == -1)
		printf("socket() error");

	memset(&mt_serv_addr, 0, sizeof(mt_serv_addr));
	mt_serv_addr.sin_family = AF_INET;
	mt_serv_addr.sin_addr.s_addr = inet_addr(mt_ip);
	mt_serv_addr.sin_port = htons(mt_port);

	if (bind(mt_sock, (struct sockaddr *)&mt_serv_addr, sizeof(mt_serv_addr)) == -1)
	{
		printf("bind err");
		close(mt_sock);
		exit(1);
	}

	mt_join_adr.imr_multiaddr.s_addr = inet_addr(mt_ip);
	mt_join_adr.imr_interface.s_addr = htonl(INADDR_ANY);

	setsockopt(mt_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mt_join_adr, sizeof(mt_join_adr));

	int mt_str_len;

	char mt_buf[1024];
	memset(&mt_from_addr, 0, sizeof(mt_from_addr));
	int adr_sz;

	int opcode_from_s;
	int port_from_s;
	char ip_from_s[20];

	while (1)
	{
		memset(&mt_buf, 0, sizeof(mt_buf));
		adr_sz = sizeof(mt_from_addr);
		mt_str_len = recvfrom(mt_sock, mt_buf, sizeof(mt_buf), 0, (struct sockaddr *)&mt_from_addr, &adr_sz);
		mt_buf[mt_str_len] = 0;
		parseCMInfo(mt_buf, &opcode_from_s, &port_from_s, ip_from_s);
		if (opcode_from_s == 1000)
		{
			printf("Server Information: %d %d %s\n", opcode_from_s, port_from_s, ip_from_s);
			break;
		}
		else
		{
			printf("Finding Server...\n");
		}
	}

	close(mt_sock);

	char nickname[30];
	char password[30];

	printf("Input Nickname: ");
	scanf("%s", nickname);
	printf("Input password: ");
	scanf("%s", password);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		printf("socket() error");
		exit(0);
	}

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(ip_from_s);
	serv_adr.sin_port = htons(port_from_s);

	if (connect(sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) == -1)
	{
		printf("connect() error!");
		exit(0);
	}
	else
		printf("Connected...........\n");

	initialHandshakeRequest(nickname, password, message);
	write(sock, message, strlen(message));

	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	FD_SET(sock, &readfds);
	printf("\x1b[2J\x1b[s");
	while (1)
	{

		allfds = readfds;

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
			if (target == -1)
			{
				if ((str_len = read(0, message, BUF_SIZE)) > 0)
				{
					target = atoi(message);
				}
				printList(store);
				printonlineusers(u_data);
				if (fetchUser(u_data, target) == -1)
				{
					printf("User Not Exists!\n");
					target = -1;
					printInput(target, u_data);
					continue;
				}
				printInput(target, u_data);
			}
			else
			{
				if ((str_len = read(0, message, BUF_SIZE)) > 0)
				{
					memset(temp, 0, 1024);

					printf("%s\n", message);
					if (!strcmp(message, "Q\n") || !strcmp(message, "q\n"))
					{
						target = -1;
						printList(store);
						printonlineusers(u_data);
						printInput(target, u_data);
						continue;
					}
					else
					{
						msgRequest(target, message, temp);
						write(sock, temp, strlen(temp));
						sprintf(temp, "\x1b[95m[ Me ] -> %s : %s", u_data[fetchUser(u_data, target)].nick, message);
						storeList(store, temp);
						printList(store);
						printonlineusers(u_data);
						printInput(target, u_data);
					}
				}
			}
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
					memset(message, 0, sizeof(message));
					sprintf(message, "\x1b[94m[%d] %s : %s", sender, u_data[fetchUser(u_data, sender)].nick, temp);
					storeList(store, message);
					printList(store);
					printonlineusers(u_data);
					printInput(target, u_data);
					break;
				case 0x9:
					parseUserList(res.data, u_data);
					//printf("\n User data Retrieved \n");
					printonlineusers(u_data);
					printInput(target, u_data);
					break;
				case 0xF:
					// Server Error ACK
					/*
							Errno - 4 byte Message - ~ 1012 bytes
							Opcode(4) Length(4) Data( Errno(4) Data(1012) )
					*/

					// TODO

					parseOpcode(res.data, &errno);
					switch (errno)
					{
					case 0x0:
						printf("\n\nLogin Failed!\n\nCheck your ID and PWD and try again!\n");
						exit(1);
						break;
					default:
						printf("An error has occured. errno: %d\n", errno);
						exit(1);
						break;
					}
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

void printonlineusers(userdata *arr)
{
	printf("\x1b[H\x1b[0m\x1b[K\n\x1b[KCurrently Online:\n\x1b[K----------------------\n");
	for (int i = 0; i < 30; i++)
	{
		if (arr[i].connected == 2)
		{
			printf("\x1b[K[%d] %s \x1b[32m●  Online\x1b[0m\n", arr[i].sockfd, arr[i].nick);
		}
		else if (arr[i].connected == 1)
		{
			printf("\x1b[K[-] %s \x1b[31m●  Offline\x1b[0m\n", arr[i].nick);
		}
	}
	printf("\x1b[K----------------------\n\x1b[B");
}

int fetchUser(userdata *data, int sock)
{
	for (int i = 0; i < 30; i++)
	{
		if (data[i].sockfd == sock && data[i].connected == 2)
			return i;
	}
	return -1;
}

void storeList(char (*store)[1024], char *str)
{
	if (!strcmp("\n", str))
		return;
	for (int i = 29; i > 0; i--)
	{
		memcpy(&store[i], &store[i - 1], 1024);
	}
	memcpy(store, str, 1024);
}

void printList(char (*store)[1024])
{
	printf("\x1b[2J\x1b[s");
	for (int i = 29; i >= 0; i--)
	{
		if (store[i] != 0)
			printf("%s", store[i]);
	}
	printf("\x1b[B");
}

void printInput(int target, userdata *data)
{
	printf("\x1b[u\x1b[B");
	if (target == -1)
	{
		printf("Input target:\n");
	}
	else
	{
		printf("To %s : \n", data[fetchUser(data, target)].nick);
	}
}