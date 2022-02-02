#ifndef JSREAD
#define JSREAD

#include <process.h>
#include "jscontrol.h"
#include "jswrite.h"

void js_rudp_recvfrom(int sock, struct sockaddr_in* clnt_addr);
//��ü���� �ڵ������� �ʿ�.
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

	WaitForSingleObject(udp_order_Mutex, INFINITE);
	js_rudp_sendpacket(hostSock, &serv_addr, clntcnt, udp_order, HOST, OK);//������ �ȵǰ� ����.
	ReleaseMutex(udp_order_Mutex);
	puts("\n###\nStart Hosting...\n###\n");

	while (isHStun)//�̴ܰ���� ����.
	{
		//ȣ��Ʈ ���� recv�Լ��� �ʿ�. ���������� �����ߴ� �͵� �ٽ� ������ ����ֱ�//������ recv������ ���� ��ü�� �Ǵ����� �ʾ���. �̹��� �ֱ�.
		js_rudp_recvfrom(hostSock, &host_clnt_addr);//Ȯ�强�� ����ؼ� �ڵ�. ���� ����ȭ ���� x

		printf("\n###\nHostUdp received from: %s\n###\n", inet_ntoa(host_clnt_addr.sin_addr));
	}

	return;
}

unsigned WINAPI Clnt_Send_Msg(void* arg)
{
	//js_rudp_sendpacket(uSocket, &host_addr, clntcnt, p2p_order, CLIENT, MESSAGE);

	return;
}

unsigned WINAPI HandleClient(void* arg)//�������� ������ �����ּ� ó���ϸ鼭 �ľ��ϱ�.
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

	while (isCStun)//�����߻� ���ѹݺ���Ȳ.
	{
		js_rudp_recvfrom(uSocket, &clnt_clnt_addr);//���⼭ ��ȣ�� �޾Ƹ���.

		printf("\n###\nClientUdp received from: %s\n###\n", inet_ntoa(clnt_clnt_addr.sin_addr));
	}

	return;
}


//�����̼��� ��ȯ �Լ� �����
int js_udp_decode(char msg_buf[])//�����˻� ���� �ڵ� �����ϱ�
{
	printf("\n###\nudp_decode Recive Data:\nClntcnt: %d, Order: %d\nFrom: %d, Type: %d\n###\n", msg_buf[1], msg_buf[2], msg_buf[3], msg_buf[4]);
	//�����ʿ�
	switch (msg_buf[3]) {
	case SERVER:
		switch (msg_buf[4]) {
		case MESSAGE:
			//�Լ�ȣ��
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
			//�Լ�ȣ��
			break;
		case OK:
			puts("\n###\nGet OK\n###\n");
			break;
		case HSTUN:
			puts("\n###\nGet HSTUN\n###\n");
			break;
		default:
			return -1;
			break;
		}
		break;
	case CLIENT:
		switch (msg_buf[4]) {
		case MESSAGE:
			//�Լ�ȣ��
			break;
		case OK:
			puts("\n###\nGet OK\n###\n");
			break;
		case CSTUN:
			puts("\n###\nGet CSTUN\n###\n");
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

void js_rudp_recvfrom(int sock, struct sockaddr_in* clnt_addr)
{
	int i;
	char udp_buf[UDP_BUF_SIZE];
	int adr_sz = sizeof(*clnt_addr);;

	for (i = 0; i < 5; i++)//���� �õ� Ƚ��
	{
		recvfrom(sock, udp_buf, UDP_BUF_SIZE, 0, (struct sockaddr*)clnt_addr, &adr_sz);

		WaitForSingleObject(udp_order_Mutex, INFINITE);

		if (udp_buf[2] < udp_order)//��� ���ŵǰ� ����.
		{
			js_udp_ssendto(sock, clnt_addr, CLIENT, OK, &udp_buf[2], sizeof(char));

			printf("\n###\nAlready recived\nNow Order: %d Get Order: %d\n###\n", udp_order, udp_buf[2]);
		}
		else if (udp_buf[2] == udp_order)
		{
			if (js_udp_decode(udp_buf) != -1)//���� �˻�. ���߿� ��ȭ�ϱ�
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

	printf("\n###\nRecive Data:\nClntcnt: %d, Order: %d\nFrom: %d, Type: %d\n###\n", udp_buf[1], udp_buf[2], udp_buf[3], udp_buf[4]);

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
	memcpy(&host_addr, (SOCKADDR_IN*)arg, sizeof(SOCKADDR_IN));//�˻��ϱ� ȣ��Ʈ ip�� �ƴѵ���.
	puts("\n###\nTry CStun...\n###\n");

	WaitForSingleObject(udp_order_Mutex, INFINITE);
	js_rudp_sendpacket(uSocket, &serv_addr, clntcnt, udp_order, CLIENT, OK);//���� ������ ������ �޾Ƹ���.
	ReleaseMutex(udp_order_Mutex);

	p2p_order = 0;
	printf("\n###\nGet Host ADDR\nIP: %s Port: %d\n###\n", inet_ntoa((*((SOCKADDR_IN*)arg)).sin_addr), ntohs((*((SOCKADDR_IN*)arg)).sin_port));//�߸� ����ް��־���.
	printf("\n###\nGet Host ADDR\nIP: %s Port: %d\n###\n", inet_ntoa((host_addr.sin_addr)), ntohs((host_addr.sin_port)));
	//p2p���ؽ� �����
	js_rudp_sendpacket(uSocket, &host_addr, clntcnt, p2p_order, CLIENT, CSTUN);//������Ʈ1. �ٸ� �����忡�� ������ �������. �������� ���� �ϱ�

	isCStun = 1;

	puts("\n###\nCStun Success!\n###\n");
}

unsigned WINAPI tcp_server_hstun(void* arg)//hstun�ذ� �ʿ�.
{
	memcpy(&p2p_addr[p2pcnt], (SOCKADDR_IN*)arg, sizeof(SOCKADDR_IN));//�˻��ϱ�
	puts("\n###\nTry HStun...\n###\n");
	p2p_orders[p2pcnt] = 0;

	//p2p���ؽ� �����
	js_rudp_sendpacket(hostSock, &p2p_addr[p2pcnt], clntcnt, p2p_order, HOST, HSTUN);//���� ��Ʈ 2

	isHStun = 1;

	p2pcnt++;
	puts("\n###\nHStun Success!\n###\n");
}

int js_tcp_decode(char msg_buf[])//ó���� �ʿ��� ����ü �����?
{
	switch (msg_buf[1]) {
	case SERVER:
		switch (msg_buf[2]) {
		case MESSAGE:
			//�Լ�ȣ��
			break;
		case CLNTCNT:
			tcp_server_clntcnt(&(msg_buf[3]));
			break;
		case BEHOST:
			tcp_server_behost();
			break;
		case CSTUN:
			printf("\n###\nDeCODE:\nGet Host ADDR\nIP: %s Port: %d\n###\n", inet_ntoa((*((SOCKADDR_IN*)&(msg_buf[3]))).sin_addr), ntohs((*((SOCKADDR_IN*)&(msg_buf[3]))).sin_port));//������ ���ö��� �ٸ�.
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
			//�Լ�ȣ��
			break;
		default:
			return -1;
			break;
		}
		break;
	case CLIENT:
		switch (msg_buf[2]) {
		case MESSAGE:
			//�Լ�ȣ��
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
