// EchoClient.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
//
#include "stdafx.h"
#include <stdio.h>
#include<process.h> 
#include<stdlib.h> 
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") 
#define BUFSIZE 1024

SOCKET sock = NULL;
unsigned char MD(void *);

class packet_base {
public:
	int head;
	int size;

	int GetPacketSize()
	{
		return sizeof(packet_base) + size;
	}
};

class packet_msg : public packet_base
{
	char msg[BUFSIZE];

	void InitHead()
	{
		head = 1;
		size = 0;
	}
public:
	void SetStrMsg(char *pStr)
	{
		InitHead();
		strcpy(msg, pStr);
		size = strlen(pStr) + 1;

		if (msg[size - 2] == '\n')
		{
			msg[size - 2] = 0;
			size -= 1;
		}
	}

	void SetBin(char *pByte, int nSize)
	{

	}
};



void PrintHexaNAscii(unsigned char* ucP, unsigned int iSize)
{
	int	iCnt;
	int	iLoop;		// Hexa code 
	int	iEndline;	// number of character in last line

	iEndline = iSize % 16;

	printf("---------------------------------------"
		"----------------------------------------\n");
	printf(" Address                         "
		"Hexa                              ASCII\n");
	printf("           ");
	for (iCnt = 0; 16>iCnt; ++iCnt)
	{
		printf("%02X ", iCnt);
	}
	putchar('\n');
	printf("---------------------------------------"
		"----------------------------------------\n");
	if (0 == iSize % 16)
	{
		iSize = iSize / 16;
	}
	else
	{
		iSize = (iSize / 16) + 1;
	}

	for (iLoop = 0; iSize>iLoop; ++iLoop)
	{
		printf("%08X   ", ucP);

		// condition of Last line
		if ((iEndline != 0) && (iLoop == iSize - 1))
		{
			for (iCnt = 0; iEndline>iCnt; ++iCnt)
			{
				printf("%02X ", *(ucP + iCnt));
			}
			for (iCnt = 0; 16 - iEndline>iCnt; ++iCnt)
			{
				printf("   ");
			}
			printf("  ");
			for (iCnt = 0; iEndline>iCnt; ++iCnt)
			{
				if (0 == *(ucP + iCnt))
				{
					printf(".");
				}
				else if (32>*(ucP + iCnt))
				{
					printf(".");
				}
				else if (127<*(ucP + iCnt))
				{
					printf(".");
				}
				else
				{
					printf("%c", *(ucP + iCnt));
				}
			}
			putchar('\n');
			break;
		}

		// normal line
		for (iCnt = 0; 16>iCnt; ++iCnt)
		{
			printf("%02X ", *(ucP + iCnt));
		}
		printf("  ");
		for (iCnt = 0; 16>iCnt; ++iCnt)
		{
			if (0 == *(ucP + iCnt))
			{
				printf(".");
			}
			else if (32>*(ucP + iCnt))
			{
				printf(".");
			}
			else if (127<*(ucP + iCnt))
			{
				printf(".");
			}
			else
			{
				printf("%c", *(ucP + iCnt));
			}
		}


		putchar('\n');
		ucP = ucP + 16;
	}
	return;

}

int my_send(
	__in SOCKET s,
	__in_bcount(len) const char FAR * buf,
	__in int len,
	__in int flags
	)
{
	int ret = send(s, buf, len, flags);

	PrintHexaNAscii((unsigned char*)buf, len);

	return ret;
}

unsigned WINAPI ThreadFunction(void* arg)
{
	char *buf = (char*)malloc(sizeof(char)*BUFSIZE);

	Sleep(1000);
	if (sock != NULL)
	{
		for (int i = 0; i < 100; i++)
		{
			memset(buf, 0, BUFSIZE);
			int recv_ret = recv(sock, buf, BUFSIZE, 0);
			if (recv_ret <= 0)
			{
				printf("접속이 종료됐습니다.\n");
				break;
			}
			else if (recv_ret >= 0)
			{
				printf("[system]:%s\n", buf);
			}
		}
	}

	return 0;
}
int main(int argc, char **argv)
{
	WSADATA wsaData;
	struct sockaddr_in addr;
	HANDLE hThread;
	DWORD dwThreadID;

	char *buf = (char*)malloc(sizeof(char)*BUFSIZE);
	char rbuf[BUFSIZE];

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
	{
		return 1;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return 1;
	}
	memset((void *)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(6002);
	hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, NULL, 0, (unsigned*)&dwThreadID);


	
	if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		return 1;
	}


	while (1)
	{
		packet_msg arr_packet_test;
		memset(buf, 0, BUFSIZE);
		fgets(buf, BUFSIZE - 1, stdin);
		if (strcmp(buf, "x\n") == 0)
		{
			printf("클라종료\n");
			break;
		}
		arr_packet_test.SetStrMsg(buf);
		send(sock, (char*)&arr_packet_test, arr_packet_test.GetPacketSize(), 0);
	}


	Sleep(1000);
	shutdown(sock, 0);
	closesocket(sock);
	WSACleanup();
	return 0;
}