// EchoServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�. 
// 


#include "stdafx.h"  


#include "stdafx.h"  
#include<process.h>  
#include<stdlib.h>  
#include<io.h>  
#include<stdio.h>  
#include<winsock2.h>  
#include <stdarg.h>
#include <string.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")  
#pragma comment(lib,"winmm.lib")
#define BUFSIZE 1024  

#define HEAD_FILE_TRANS = 1; 
#define HEAD_MESSAGE = 0; 


typedef struct g_socket_session {
	int n_last_head;
	int n_end_byte;
	int n_start_byte;
	int n_last_set_head_byte;
	int n_socket;
	char str_name[20];
	int n_currency;
	int n_sniffing;
	int n_bating_currency;
}SocketSession;


struct sockaddr_in server, client;
SOCKET socket_listen, new_socket[128] = { NULL, };
int arr_is_use_socket[128] = { 0, };
SocketSession g_socket_session[128];
int n_last_socket = 0;
int g_state_room = 0;
SOCKET arr_bating_accept_socket[128] = {0, };

void initSessionHeader(SocketSession *pSession)
{
	memset(pSession, 0, 20);
}

void displaySession(SocketSession *pSession)
{
	/*
	printf(""
	"n_last_head = %d;\n"
	"n_end_byte = %d;\n"
	"n_start_byte = %d;\n"
	"n_state = %d;\n"
	"n_last_set_head_byte = %d;\n",
	pSession->n_last_head,
	pSession->n_end_byte,
	pSession->n_start_byte,
	pSession->n_state,
	pSession->n_last_set_head_byte
	);
	*/
}

void debug_printf(char *fmt, ...)
{
	/*
	char buf[512] = {0,};
	va_list ap;

	strcpy(buf, "[debug] ");
	va_start(ap, fmt);
	vsprintf(buf + strlen(buf), fmt, ap);
	va_end(ap);

	printf(buf);
	*/
}


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


bool IsBatingUser(SOCKET socket)
{
	for (int i = 0; i < sizeof(arr_bating_accept_socket)/4; i++)
	{
		if (arr_bating_accept_socket[i] == socket)
		{
			return true;
		}
	}

	return false;
}


void SetBatingUser(SOCKET socket)
{
	for (int i = 0; i < sizeof(arr_bating_accept_socket)/4; i++)
	{
		if (arr_bating_accept_socket[i] == 0)
		{
			arr_bating_accept_socket[i] = socket;
			break;
		}
	}
}

unsigned WINAPI ThreadFunction(void* arg)
{
	int recv_count;
	SOCKET socket = *(SOCKET*)arg;
	int n_socket_num;

	for (int i = 0; i < 128; i++)
	{
		if (socket == new_socket[i])
		{
			n_socket_num = i;
			break;
		}
	}

	char str_buf[BUFSIZE];
	char message[BUFSIZE];
	printf("[debug] => %d", n_socket_num);
	arr_is_use_socket[n_socket_num] = 1;
	memset(message, 0, BUFSIZE);
	SocketSession *session = &g_socket_session[n_socket_num];
	initSessionHeader(session);
	while ((recv_count = recv(socket, message, BUFSIZE, 0)) != 0)
	{
		if (recv_count == -1)
		{
			printf("%d ���� ����\n", n_socket_num);
			break;
		}
		debug_printf("%d Ŭ���̾�Ʈ ����\n", n_socket_num);
		// ������ �Ϸ�Ƴ� ���� 
		int n_left_byte = recv_count;
		printf("======> ���۵� ����Ʈ %d\n", n_left_byte);


		//PrintHexaNAscii((unsigned char*)message, recv_count);

		for (int i = 0; i < n_left_byte; i++)
		{
			// ��� ���� ������ �ȵƴٸ� ������ ���� 
			// HEADER : HEADER(4byte) + SIZE(4byte) + DATA(???) 
			if (session->n_last_set_head_byte < 8)
			{
				int n_size_head_write = 8 - session->n_last_set_head_byte;
				if (n_left_byte < n_size_head_write)
				{
					n_size_head_write = n_left_byte;
				}

				debug_printf("��� write start : %d\n", n_size_head_write);
				memcpy(
					(char*)(session)+session->n_last_set_head_byte,
					message + recv_count - n_left_byte,
					n_size_head_write
					);
				displaySession(session);
				session->n_last_set_head_byte += n_size_head_write;
				n_left_byte -= n_size_head_write;
				debug_printf("��� write end %d \n", n_size_head_write);
				displaySession(session);
			}

			// ����� �ϼ��� ���¶�� �۾��� ������ 
			if (session->n_last_set_head_byte >= 8)
			{
				debug_printf("�۾� ����\n");
				displaySession(session);
				// �˼����� ������ ������ �ʱ�ȭ �ϰ� �ٽ� ���� 
				if (session->n_last_head > 1)
				{
					debug_printf("��� ����\n");
					initSessionHeader(session);
				}

				int n_size_write_content = session->n_end_byte - session->n_start_byte;
				if (n_size_write_content > n_left_byte)
				{
					n_size_write_content = n_left_byte;
				}
				memcpy(
					str_buf + session->n_start_byte,
					message + recv_count - n_left_byte,
					n_size_write_content
					);
				//debug_printf("[buf - size:%d]%s\n\n", n_size_write_content, str_buf);


				session->n_start_byte += n_size_write_content;
				n_left_byte -= n_size_write_content;
			}

			// ��� �������� ���۹޾ҳ� �˻� 
			if (session->n_last_set_head_byte >= 8 && session->n_end_byte == session->n_start_byte)
			{
				char send_str[1024] = { 0, };
				char *pMeesage = str_buf;
				debug_printf("���� �� ��� �ʱ�ȭ\n");
				// ������ �� �޾Ҵٸ� ��������� �ʱ�ȭ 
				displaySession(session);
				initSessionHeader(session);
				printf("pMessage = %c", pMeesage[0]);
				char header = pMeesage[0];
				if (header == '|')
				{
					send(socket, "�����ϽŰ� ȯ���մϴ�.", strlen("�����ϽŰ� ȯ���մϴ�.") - 1, 0);
				}

				if (pMeesage[0] == '1')
				{
					char temp_id[20];
					strcpy(temp_id, pMeesage + 2);
					strcpy(session->str_name, pMeesage + 2);
					session->n_currency = 10000;
					sprintf(send_str, "[%s]�� ȯ���մϴ�. 10,000���� �����߽��ϴ�. ���ӿ� �����Ϸ��� '2'�� �Է����ּ���", temp_id);
					send(socket, send_str, strlen(send_str) + 1, 0);
				}


				if (pMeesage[0] == '0')
				{
					char temp_id[20];
					strcpy(temp_id, pMeesage + 2);
					sprintf(send_str, "[%s]���� �����Ͻ� ��ȭ�� %d �Դϴ�.", session->str_name, session->n_currency);
					send(socket, send_str, strlen(send_str) + 1, 0);
				}

				if (pMeesage[0] == '2')
				{
					if (g_state_room == 1)
					{
						sprintf(send_str, "������ ���� ���Դϴ�. ��� �� �������ּ���.");
					}
					else
					{
						sprintf(send_str, "�����ƽ��ϴ�. ������ �Ϸ��� 3:[Ȧ:0,¦:1]:[�ݾ�] �� �Է����ּ���");
						SetBatingUser(socket);
					}

					send(socket, send_str, strlen(send_str) + 1, 0);
				}

				if (pMeesage[0] == '3')
				{
					if (g_state_room == 0)

					{
						sprintf(send_str, "������ ����Ǿ� ������ �� �����ϴ�.");
					}
					else if (!IsBatingUser(socket))
					{
						sprintf(send_str, "���ÿ� ������û�� ���߽��ϴ�. ���� ��ȸ�� �븮����");
					}
					else if(g_state_room == 1 && IsBatingUser(socket))
					{
						if (pMeesage[2] == '0')
						{
							session->n_sniffing = 0;
						}
						else if (pMeesage[2] == '1')
						{
							session->n_sniffing = 1;
						}

						// 3:1:30000

						// ����Ʈ �ļ�
						int n_bating_currency = atoi(pMeesage+4);

						if(n_bating_currency > session->n_currency)
						{
							sprintf(send_str, "�����Ͻ� ��ȭ���� ���þ��� �� Ů�ϴ�. �����Ͻ� ��ȭ : %d, ���þ� : %d", session->n_currency ,session->n_bating_currency);
						}
						else
						{
							session->n_bating_currency = n_bating_currency;
							session->n_currency -= n_bating_currency;
							sprintf(send_str, "�����߽��ϴ�. %d(0:Ȧ,¦:1)�� %d�� �ẕ̇���!!! �αٵαٵα�!!!!!!!", session->n_sniffing, session->n_currency + n_bating_currency);
						}
						
						
					}

					send(socket, send_str, strlen(send_str) + 1, 0);
				}


				printf("===========================\n", str_buf);
				printf("[���ۿϷ� %d]      ���޹��� => %s\n", n_socket_num, str_buf);
				printf("===========================\n\n", str_buf);
				memset(str_buf, 0, BUFSIZE);
			}

			// ó���� ��Ŷ�� ������ recv ����ϱ� ���� Ż�� 
			if (n_left_byte <= 0)
			{
				break;
			}
		}

		memset(message, 0, BUFSIZE);
	}

	closesocket(socket);
	new_socket[n_socket_num] = NULL;
	arr_is_use_socket[n_socket_num] = 0;

	return 0;
}

SOCKET* GetNewSocket()
{
	for (int i = 0; i < 128; i++)
	{
		if (arr_is_use_socket[i] == 0)
		{
			return &new_socket[i];
		}
	}

	return NULL;
}

int GetActiveSocketCount()
{
	int sum = 0;
	for (int i = 0; i < 128; i++)
	{
		if (new_socket[i] != NULL)
		{
			sum += 1;
		}
	}

	return sum;
}

void SendMessageAll(char *pMsg)
{
	for (int i = 0; i <= 128; i++)
	{
		if (arr_is_use_socket[i] != 0)
		{
			send(new_socket[i], pMsg, strlen(pMsg) + 1, 0);
		}
	}
}

void SendMessageBatingUser(char *pMsg)
{
	for (int i = 0; i < sizeof(arr_bating_accept_socket)/4; i++)
	{
		if (arr_bating_accept_socket[i] != 0)
		{
			send(new_socket[i], pMsg, strlen(pMsg) + 1, 0);
		}
	}
}


void SendMessageNotBatingUser(char *pMsg)
{
	for (int i = 0; i < sizeof(arr_bating_accept_socket)/4; i++)
	{
		if (arr_bating_accept_socket[i] == 0)
		{
			send(new_socket[i], pMsg, strlen(pMsg) + 1, 0);
		}
	}
}

void ClearBatingUser()
{
	for (int i = 0; i < sizeof(arr_bating_accept_socket)/4; i++)
	{
		arr_bating_accept_socket[i] = 0;
	}
}

void process_bating_result(int odd)
{
	odd = odd % 2;
	char msg_buf[200];

	for (int i = 0; i <= 128; i++)
	{
		// ������� ���Ͼƴϸ� �н�
		if (arr_is_use_socket[i] == 0)
		{
			continue;
		}

		// ���������� �ƴϸ� �н�
		if (!IsBatingUser(new_socket[i]))
		{
			continue;
		}

		if (odd == g_socket_session[i].n_sniffing)
		{
			g_socket_session[i].n_currency += g_socket_session[i].n_bating_currency * 2;
			sprintf(msg_buf, "�����մϴ�. ����� �¸��ؼ� %d���� ȹ���߽��ϴ�. �� ��ȭ�� : %d �Դϴ�.", g_socket_session[i].n_bating_currency, g_socket_session[i].n_currency);
			send(new_socket[i], msg_buf, strlen(msg_buf) + 1, 0);
		}
		else
		{
			sprintf(msg_buf, "���ϴ�. �ʴ� �й��ؼ� %d���� �Ҿ���. �� ��ȭ�� : %d �Դϴ�.", g_socket_session[i].n_bating_currency, g_socket_session[i].n_currency);
			send(new_socket[i], msg_buf, strlen(msg_buf) + 1, 0);
		}
	}
}


DWORD GetSystemSecond()
{
	return timeGetTime() / 1000;
}

unsigned WINAPI ProcessGame(void *arg)
{
	int run_room = 0;
	int time_bating = 0;
	int time_random = 0;
	int prev_time = GetSystemSecond();
	char buf_message[128];


	static int time_waiting_start = prev_time + 12;
	static int time_bating_start;

	srand(time(NULL));
	while (1)
	{
		Sleep(100);
		int sys_time = GetSystemSecond();
		int dt = sys_time - prev_time;

		// ���� ���� ���� ���
		{
			static int time_dt_connect_sum = 0;
			time_dt_connect_sum += dt;
			if (time_dt_connect_sum >= 5)
			{
				printf("���� ���� ���� : %d\n", GetActiveSocketCount());
				time_dt_connect_sum = 0;
			}
		}
		
		if (g_state_room == 0)
		{

			if ((time_waiting_start - sys_time) % 3 == 0 && dt == 1)
			{
				printf("���� ���۱��� %d�� ���ҽ��ϴ�\n", time_waiting_start - sys_time);
				char buf[256];
				sprintf(buf, "���� ���۱��� %d�� ���ҽ��ϴ�\n", time_waiting_start - sys_time);
				SendMessageNotBatingUser(buf);
			}

			if (time_waiting_start - sys_time  <= 0)
			{
				time_bating_start = sys_time + 12;
				g_state_room = 1;
				SendMessageAll("������ �����ƽ��ϴ�. �������� ���� �е��� ���� ��ȸ�� �������ּ���");
				SendMessageBatingUser("���� ������ ������ �ݰ����ϴ�. 12�� �ȿ� �������ּ���.");
			}
		}
		else if (g_state_room == 1)
		{
			if ((time_bating_start - sys_time) % 3 == 0 && dt == 1)
			{
				printf("�ֻ��� ��������� %d�� ���ҽ��ϴ�\n", time_bating_start - sys_time);
			}

			if (time_bating_start - sys_time <= 0)
			{
				int random_1, random_2, random_3;
				random_1 = rand() % 6 + 1;
				random_2 = rand() % 6 + 1;
				random_3 = rand() % 6 + 1;

				printf("�ֻ��� ��� %d %d %d �Դϴ�\n", random_1, random_2, random_3);

				g_state_room = 0;
				time_waiting_start = prev_time + 12;

				// ���� ���μ���
				process_bating_result(random_1+random_2+random_3);
				
				SendMessageAll("������ �������ϴ�. ������û���ּ���.");
				ClearBatingUser();
				continue;
			}
		}

		prev_time = sys_time;
	}
}

int main(int argc, char *argv[])
{
	WSADATA wsa;
	char message[BUFSIZE] = "ping!\n";
	HANDLE hThread;
	DWORD dwThreadID;

	n_last_socket = 0;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("WSAStartup ���� %d", WSAGetLastError());
		return 1;
	}

	// ���� ����  
	if ((socket_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("���Ͽ��� : %d", WSAGetLastError());
	}

	printf("���ϻ��� .\n");

	// ���ε��� ����ü �ʱ�ȭ  
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(6002);

	// ���Ͽ� ���� ���� ���ε�  
	if (bind(socket_listen, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("���ε� ���� %d", WSAGetLastError());
	}

	//Ŭ�� ���� �� �ֵ��� ��  
	printf("���� ��� ��\n");
	listen(socket_listen, 127);
	printf("���� ���� ����\n");
	int c;
	n_last_socket = 0;

	_beginthreadex(NULL, 0, ProcessGame, socket, 0, (unsigned*)&dwThreadID);
	while (true)
	{
		int *pSocketNum = (int*)malloc(sizeof(int));
		c = sizeof(struct sockaddr_in);


		SOCKET temp_socket = accept(socket_listen, (struct sockaddr *)&client, &c);
		SOCKET *socket = GetNewSocket();
		*socket = temp_socket;
		if (*socket == INVALID_SOCKET)
		{
			printf("accept ���� : %d", WSAGetLastError());
		}
		printf("������ : %d\n", GetActiveSocketCount());
		if (GetActiveSocketCount() >= 5)
		{
			strcpy(message, "�����ڰ� �� á���ϴ�. bye");
			printf("�� á���� ���� �õ�\n");

			// �����޼��� ���� 
			send(*socket, message, strlen(message) + 1, 0);

			// ���� 
			shutdown(*socket, 0);
			closesocket(*socket);
			*socket = NULL;

			continue;
		}
		send(temp_socket, "�����ϽŰ� ȯ���մϴ�. �̸��� ���Ϸ��� 1:[�̸�] �Է��ϼ���", strlen("�����ϽŰ� ȯ���մϴ�. �̸��� ���Ϸ��� 1:[�̸�] �Է��ϼ���") +1,  0);
		// �����忡�� listen ����� 
		hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunction, socket, 0, (unsigned*)&dwThreadID);
	}
	for (int i = 0; i < GetActiveSocketCount(); i++)
	{
		closesocket(new_socket[i]);
	}

	closesocket(socket_listen);
	WSACleanup();

	return 0;
}