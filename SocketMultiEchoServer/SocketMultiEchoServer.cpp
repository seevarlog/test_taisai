// EchoServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다. 
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
			printf("%d 접속 단절\n", n_socket_num);
			break;
		}
		debug_printf("%d 클라이언트 전송\n", n_socket_num);
		// 전송이 완료됐나 조사 
		int n_left_byte = recv_count;
		printf("======> 전송된 바이트 %d\n", n_left_byte);


		//PrintHexaNAscii((unsigned char*)message, recv_count);

		for (int i = 0; i < n_left_byte; i++)
		{
			// 헤더 정보 전송이 안됐다면 헤드부터 셋팅 
			// HEADER : HEADER(4byte) + SIZE(4byte) + DATA(???) 
			if (session->n_last_set_head_byte < 8)
			{
				int n_size_head_write = 8 - session->n_last_set_head_byte;
				if (n_left_byte < n_size_head_write)
				{
					n_size_head_write = n_left_byte;
				}

				debug_printf("헤더 write start : %d\n", n_size_head_write);
				memcpy(
					(char*)(session)+session->n_last_set_head_byte,
					message + recv_count - n_left_byte,
					n_size_head_write
					);
				displaySession(session);
				session->n_last_set_head_byte += n_size_head_write;
				n_left_byte -= n_size_head_write;
				debug_printf("헤더 write end %d \n", n_size_head_write);
				displaySession(session);
			}

			// 헤더가 완성된 상태라면 작업을 시작함 
			if (session->n_last_set_head_byte >= 8)
			{
				debug_printf("작업 시작\n");
				displaySession(session);
				// 알수없는 헤더라면 세션을 초기화 하고 다시 시작 
				if (session->n_last_head > 1)
				{
					debug_printf("헤더 오류\n");
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

			// 모든 컨텐츠를 전송받았나 검사 
			if (session->n_last_set_head_byte >= 8 && session->n_end_byte == session->n_start_byte)
			{
				char send_str[1024] = { 0, };
				char *pMeesage = str_buf;
				debug_printf("전송 끝 헤더 초기화\n");
				// 전송을 다 받았다면 세션헤더를 초기화 
				displaySession(session);
				initSessionHeader(session);
				printf("pMessage = %c", pMeesage[0]);
				char header = pMeesage[0];
				if (header == '|')
				{
					send(socket, "접속하신걸 환영합니다.", strlen("접속하신걸 환영합니다.") - 1, 0);
				}

				if (pMeesage[0] == '1')
				{
					char temp_id[20];
					strcpy(temp_id, pMeesage + 2);
					strcpy(session->str_name, pMeesage + 2);
					session->n_currency = 10000;
					sprintf(send_str, "[%s]님 환영합니다. 10,000원을 지급했습니다. 게임에 참여하려면 '2'를 입력해주세요", temp_id);
					send(socket, send_str, strlen(send_str) + 1, 0);
				}


				if (pMeesage[0] == '0')
				{
					char temp_id[20];
					strcpy(temp_id, pMeesage + 2);
					sprintf(send_str, "[%s]님의 보유하신 재화는 %d 입니다.", session->str_name, session->n_currency);
					send(socket, send_str, strlen(send_str) + 1, 0);
				}

				if (pMeesage[0] == '2')
				{
					if (g_state_room == 1)
					{
						sprintf(send_str, "게임이 진행 중입니다. 잠시 후 참여해주세요.");
					}
					else
					{
						sprintf(send_str, "참가됐습니다. 배팅을 하려면 3:[홀:0,짝:1]:[금액] 을 입력해주세요");
						SetBatingUser(socket);
					}

					send(socket, send_str, strlen(send_str) + 1, 0);
				}

				if (pMeesage[0] == '3')
				{
					if (g_state_room == 0)

					{
						sprintf(send_str, "게임이 종료되어 배팅할 수 없습니다.");
					}
					else if (!IsBatingUser(socket))
					{
						sprintf(send_str, "배팅에 참가신청을 안했습니다. 다음 기회를 노리세요");
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

						// 포인트 꼼수
						int n_bating_currency = atoi(pMeesage+4);

						if(n_bating_currency > session->n_currency)
						{
							sprintf(send_str, "보유하신 재화보다 배팅액이 더 큽니다. 보유하신 재화 : %d, 배팅액 : %d", session->n_currency ,session->n_bating_currency);
						}
						else
						{
							session->n_bating_currency = n_bating_currency;
							session->n_currency -= n_bating_currency;
							sprintf(send_str, "배팅했습니다. %d(0:홀,짝:1)에 %d를 거셨군요!!! 두근두근두근!!!!!!!", session->n_sniffing, session->n_currency + n_bating_currency);
						}
						
						
					}

					send(socket, send_str, strlen(send_str) + 1, 0);
				}


				printf("===========================\n", str_buf);
				printf("[전송완료 %d]      전달문자 => %s\n", n_socket_num, str_buf);
				printf("===========================\n\n", str_buf);
				memset(str_buf, 0, BUFSIZE);
			}

			// 처리할 패킷이 없으면 recv 대기하기 위해 탈출 
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
		// 사용중인 소켓아니면 패스
		if (arr_is_use_socket[i] == 0)
		{
			continue;
		}

		// 배팅유저가 아니면 패스
		if (!IsBatingUser(new_socket[i]))
		{
			continue;
		}

		if (odd == g_socket_session[i].n_sniffing)
		{
			g_socket_session[i].n_currency += g_socket_session[i].n_bating_currency * 2;
			sprintf(msg_buf, "축하합니다. 당신은 승리해서 %d원을 획득했습니다. 총 재화는 : %d 입니다.", g_socket_session[i].n_bating_currency, g_socket_session[i].n_currency);
			send(new_socket[i], msg_buf, strlen(msg_buf) + 1, 0);
		}
		else
		{
			sprintf(msg_buf, "딱하다. 너는 패배해서 %d원을 잃었다. 총 재화는 : %d 입니다.", g_socket_session[i].n_bating_currency, g_socket_session[i].n_currency);
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

		// 접속 중인 유저 띄움
		{
			static int time_dt_connect_sum = 0;
			time_dt_connect_sum += dt;
			if (time_dt_connect_sum >= 5)
			{
				printf("접속 중인 유저 : %d\n", GetActiveSocketCount());
				time_dt_connect_sum = 0;
			}
		}
		
		if (g_state_room == 0)
		{

			if ((time_waiting_start - sys_time) % 3 == 0 && dt == 1)
			{
				printf("게임 시작까지 %d초 남았습니다\n", time_waiting_start - sys_time);
				char buf[256];
				sprintf(buf, "게임 시작까지 %d초 남았습니다\n", time_waiting_start - sys_time);
				SendMessageNotBatingUser(buf);
			}

			if (time_waiting_start - sys_time  <= 0)
			{
				time_bating_start = sys_time + 12;
				g_state_room = 1;
				SendMessageAll("배팅이 마감됐습니다. 참여하지 못한 분들은 다음 기회에 참여해주세요");
				SendMessageBatingUser("배팅 참여자 여러분 반갑습니다. 12초 안에 배팅해주세요.");
			}
		}
		else if (g_state_room == 1)
		{
			if ((time_bating_start - sys_time) % 3 == 0 && dt == 1)
			{
				printf("주사위 굴리기까지 %d초 남았습니다\n", time_bating_start - sys_time);
			}

			if (time_bating_start - sys_time <= 0)
			{
				int random_1, random_2, random_3;
				random_1 = rand() % 6 + 1;
				random_2 = rand() % 6 + 1;
				random_3 = rand() % 6 + 1;

				printf("주사위 결과 %d %d %d 입니다\n", random_1, random_2, random_3);

				g_state_room = 0;
				time_waiting_start = prev_time + 12;

				// 정산 프로세스
				process_bating_result(random_1+random_2+random_3);
				
				SendMessageAll("게임이 끝났습니다. 참가신청해주세요.");
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
		printf("WSAStartup 에러 %d", WSAGetLastError());
		return 1;
	}

	// 소켓 만듬  
	if ((socket_listen = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("소켓에러 : %d", WSAGetLastError());
	}

	printf("소켓생성 .\n");

	// 바인딩할 구조체 초기화  
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(6002);

	// 소켓에 대한 정보 바인드  
	if (bind(socket_listen, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("바인딩 실패 %d", WSAGetLastError());
	}

	//클라가 받을 수 있도록 함  
	printf("리슨 대기 중\n");
	listen(socket_listen, 127);
	printf("리슨 상태 시작\n");
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
			printf("accept 실패 : %d", WSAGetLastError());
		}
		printf("접속자 : %d\n", GetActiveSocketCount());
		if (GetActiveSocketCount() >= 5)
		{
			strcpy(message, "접속자가 꽉 찼습니다. bye");
			printf("꽉 찼지만 접속 시도\n");

			// 에러메세지 전송 
			send(*socket, message, strlen(message) + 1, 0);

			// 종료 
			shutdown(*socket, 0);
			closesocket(*socket);
			*socket = NULL;

			continue;
		}
		send(temp_socket, "접속하신걸 환영합니다. 이름을 정하려면 1:[이름] 입력하세요", strlen("접속하신걸 환영합니다. 이름을 정하려면 1:[이름] 입력하세요") +1,  0);
		// 스레드에서 listen 대기함 
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