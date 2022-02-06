#ifndef JSREAD
#define JSREAD

#include <process.h>
#include "jscontrol.h"
#include "jswrite.h"

void js_udp_recvfrom(int sock, struct sockaddr_in* clnt_addr);

unsigned WINAPI HandleHost(void* arg)
{
	SOCKADDR_IN host_addr;
	isHStun = 0;
	p2pcnt = 0;

	hostSock = socket(PF_INET, SOCK_DGRAM, 0);
	if (hostSock == INVALID_SOCKET)
		error_handling("UDP socket creation error");
	memset(&host_addr, 0, sizeof(host_addr));
	host_addr.sin_family = AF_INET;
	host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	host_addr.sin_port = htons(HOST_PORT);

	if (bind(hostSock, (SOCKADDR*)&host_addr, sizeof(host_addr)) == SOCKET_ERROR)
		error_handling("bind() error");

	while (!isHStun)
	{
		js_udp_sendpacket(hostSock, &serv_addr, HOST, OK);
	}
	isHStun = 0;
	puts("\n###\nStart Hosting...\n###\n");

	while (1)
	{
		js_udp_recvfrom(hostSock, &host_clnt_addr);

		printf("\n###\nHostUdp received from: %s\n###\n", inet_ntoa(host_clnt_addr.sin_addr));
	}

	return;
}

unsigned WINAPI Clnt_Send_Msg(void* arg)
{
	

	return;
}

unsigned WINAPI HandleClient(void* arg)//에러나면 지우지 말고주석 처리하면서 파악하기.
{
	SOCKADDR_IN clnt_addr;

	isCStun = 0;

	uSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (uSocket == INVALID_SOCKET)
		error_handling("socket() error");
	///*
	memset(&clnt_addr, 0, sizeof(clnt_addr));
	clnt_addr.sin_family = AF_INET;
	clnt_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	clnt_addr.sin_port = htons(CLIENT_PORT);

	if (bind(uSocket, (SOCKADDR*)&clnt_addr, sizeof(clnt_addr)) == SOCKET_ERROR)
		error_handling("bind() error");
	//	*/
	_beginthreadex(NULL, 0, Clnt_Send_Msg, NULL, 0, NULL);

	puts("\n###\nStart Client...\n###\n");

	while (1)//오류발생 무한반복상황.
	{
		js_udp_recvfrom(uSocket, &clnt_clnt_addr);//여기서 신호를 받아먹음.//여기서 이상한 신호 수신함.

		printf("\n###\nClientUdp received from: %s\n###\n", inet_ntoa(clnt_clnt_addr.sin_addr));
	}

	return;
}


//릴레이서버 전환 함수 만들기
int js_udp_decode(char msg_buf[])//오류검사 관련 코드 구현하기
{
	printf("\n###\nudp_decode Recive Data:\nClntcnt: %d, Order: %d\nFrom: %d, Type: %d\n###\n", msg_buf[1], msg_buf[2], msg_buf[3], msg_buf[4]);
	//수정필요
	switch (msg_buf[3]) {
	case SERVER:
		switch (msg_buf[4]) {
		case MESSAGE:
			//함수호출
			break;
		case OK:
			puts("\n###\nGet OK\n###\n");
			break;
		default:
			return -1;
			break;
		}
		break;
	case HOST:
		switch (msg_buf[4]) {
		case MESSAGE:
			//함수호출
			break;
		case OK:
			puts("\n###\nGet OK\n###\n");
			break;
		case HSTUN:
			puts("\n###\nGet HSTUN\n###\n");
			js_udp_sendpacket(uSocket, &host_addr, CLIENT, HSTUN);
			break;
		case CSTUN:
			puts("\n###\nCStun Success!\n###\n");
			isCStun = 1;
			break;
		default:
			return -1;
			break;
		}
		break;
	case CLIENT:
		switch (msg_buf[4]) {
		case MESSAGE:
			//함수호출
			break;
		case OK:
			puts("\n###\nGet OK\n###\n");
			break;
		case CSTUN:
			puts("\n###\nGet CSTUN\n###\n");
			js_udp_sendpacket(hostSock, &p2p_addr[0], HOST, CSTUN);//주소값 수정필요
			break;
		case HSTUN:
			puts("\n###\nHStun Success!\n###\n");
			isHStun = 1;
			break;
		default:
			return -1;
			break;
		}
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

void js_udp_recvfrom(int sock, struct sockaddr_in* clnt_addr)//아직 보류
{
	int i;
	char udp_buf[UDP_BUF_SIZE];
	int adr_sz = sizeof(*clnt_addr);;

	recvfrom(sock, udp_buf, UDP_BUF_SIZE, 0, (struct sockaddr*)clnt_addr, &adr_sz);

	if (js_udp_decode(udp_buf) != -1)//오류 검사. 나중에 강화하기
		js_udp_ssendto(sock, clnt_addr, CLIENT, OK, &udp_buf[2], sizeof(char));
	else
		puts("\n###\nWrong Packet!\n###\n");

	/*
	for (i = 0; i < 5; i++)//수신 시도 횟수
	{
		recvfrom(sock, udp_buf, UDP_BUF_SIZE, 0, (struct sockaddr*)clnt_addr, &adr_sz);

		WaitForSingleObject(udp_order_Mutex, INFINITE);

		if (udp_buf[2] < udp_order)//계속 수신되고 있음.
		{
			js_udp_ssendto(sock, clnt_addr, CLIENT, OK, &udp_buf[2], sizeof(char));

			printf("\n###\nAlready recived\nNow Order: %d Get Order: %d\n###\n", udp_order, udp_buf[2]);
		}
		else if (udp_buf[2] == udp_order)
		{
			if (js_udp_decode(udp_buf) != -1)//오류 검사. 나중에 강화하기
			{
				js_udp_ssendto(sock, clnt_addr, CLIENT, OK, &udp_buf[2], sizeof(char));

				udp_order++;
				if (udp_order > MAX_UDP_ORDER)
					udp_order = 0;

				break;
			}
			else
				puts("\n###\nWrong Packet!\n###\n");
		}
		else
			puts("\n###\nWrong Order!\n###\n");

		ReleaseMutex(udp_order_Mutex);
	}
	*/

	printf("\n###\nRecive UDP Data:\nClntcnt: %d, Order: %d\nFrom: %d, Type: %d\n###\n", udp_buf[1], udp_buf[2], udp_buf[3], udp_buf[4]);

	return;
}

//TCP
//SERVER

void tcp_server_clntcnt(char* buf)
{
	clntcnt = ((int)buf[0]);

	printf("\n###\nCLNT ID: %d\n###\n", clntcnt);

	_beginthreadex(NULL, 0, HandleClient, NULL, 0, NULL);

	puts("\n###\nCreate Client thread success!\n###\n");

	return;
}

void tcp_server_behost()
{
	_beginthreadex(NULL, 0, HandleHost, NULL, 0, NULL);

	puts("\n###\nCreate Hosting thread success!\n###\n");

	return;
}

unsigned WINAPI tcp_server_cstun(void* arg)
{
	memcpy(&host_addr, (SOCKADDR_IN*)arg, sizeof(SOCKADDR_IN));
	puts("\n###\nTry CStun...\n###\n");
	isCStun = 0;

	while (!isCStun)
	{
		js_udp_sendpacket(uSocket, &serv_addr, CLIENT, OK);
		Sleep(RUDP_SLEEP);
	}
	isCStun = 0;

	p2p_order = 0;
	printf("\n###\nGet Host ADDR\nIP: %s Port: %d\n###\n", inet_ntoa((*((SOCKADDR_IN*)arg)).sin_addr), ntohs((*((SOCKADDR_IN*)arg)).sin_port));
	printf("\n###\nGet Host ADDR\nIP: %s Port: %d\n###\n", inet_ntoa((host_addr.sin_addr)), ntohs((host_addr.sin_port)));

	while (!isCStun)//isCstun==1//못받은 쪽에서는 recv에서 다시 응답하는 메시지 처리
	{
		js_udp_sendpacket(uSocket, &host_addr, CLIENT, CSTUN);
		Sleep(RUDP_SLEEP);
	}

	return 0;
}

unsigned WINAPI tcp_server_hstun(void* arg)//hstun해결 필요.
{
	memcpy(&p2p_addr[p2pcnt], (SOCKADDR_IN*)arg, sizeof(SOCKADDR_IN));
	puts("\n###\nTry HStun...\n###\n");
	isHStun = 0;

	p2p_orders[p2pcnt] = 0;

	while (!isHStun)//isHstun==1//못받은 쪽에서는 recv에서 다시 응답하는 메시지 처리
	{
		js_udp_sendpacket(hostSock, &p2p_addr[p2pcnt], HOST, HSTUN);
		Sleep(RUDP_SLEEP);
	}

	p2pcnt++;

	return 0;
}

int js_tcp_decode(char msg_buf[])//처리에 필요한 구조체 만들기?
{
	switch (msg_buf[1]) {
	case SERVER:
		switch (msg_buf[2]) {
		case MESSAGE:
			//함수호출
			break;
		case CLNTCNT:
			tcp_server_clntcnt(&(msg_buf[3]));
			break;
		case BEHOST:
			tcp_server_behost();
			break;
		case CSTUN:
			printf("\n###\nDeCODE:\nGet Host ADDR\nIP: %s Port: %d\n###\n", inet_ntoa((*((SOCKADDR_IN*)&(msg_buf[3]))).sin_addr), ntohs((*((SOCKADDR_IN*)&(msg_buf[3]))).sin_port));
			_beginthreadex(NULL, 0, tcp_server_cstun, &(msg_buf[3]), 0, NULL);
			break;
		case HSTUN:
			_beginthreadex(NULL, 0, tcp_server_hstun, &(msg_buf[3]), 0, NULL);
			break;
		default:
			return -1;
			break;
		}
		break;
	case HOST:
		switch (msg_buf[2]) {
		case MESSAGE:
			//함수호출
			break;
		default:
			return -1;
			break;
		}
		break;
	case CLIENT:
		switch (msg_buf[2]) {
		case MESSAGE:
			//함수호출
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
		break;
	}

	return 0;
}

void js_tcp_read(int len, char msg_buf[])
{
	int i;

	for (i = 0; i < len; i += msg_buf[i])
		js_tcp_decode(&msg_buf[i]);

	return;
}

#endif
