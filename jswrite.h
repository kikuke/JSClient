#ifndef JSWRITE
#define JSWRITE

#include <stdio.h>
#include <windows.h>
#include "jsnet.h"

#define RUDP_SLEEP 100//테스트 용으로 낮췄음. 원래 100임.

//나중에 비트단위로 쪼개서 보내기
int js_udp_encode(char udp_buf[], const void* msg_buf, size_t n, js_udp_struct udp_struct)
{
	udp_buf[0] = n + 6;
	udp_buf[1] = udp_struct.msg_clntcnt;
	udp_buf[2] = udp_struct.msg_order;
	udp_buf[3] = udp_struct.msg_from;
	udp_buf[4] = udp_struct.msg_type;

	memcpy(&udp_buf[5], msg_buf, n);
	if (n > UDP_BUF_SIZE - 6)
		return -1;

	udp_buf[n + 5] = '\0';

	return udp_buf[0];
}

int js_udp_sendto(int sock, struct sockaddr_in* clnt_addr, const void* msg_buf, size_t n, js_udp_struct udp_struct)
{
	char udp_buf[UDP_BUF_SIZE];

	if (js_udp_encode(udp_buf, msg_buf, n, udp_struct) == -1)
		return -1;

	return sendto(sock, udp_buf, udp_buf[0], 0, (struct sockaddr*)clnt_addr, sizeof(*clnt_addr));
}

int js_udp_ssendto(int sock, struct sockaddr_in* clnt_addr, int msg_from, int msg_type, const void* msg_buf, size_t n)
{
	js_udp_struct udp_struct;

	udp_struct.msg_from = msg_from;
	udp_struct.msg_type = msg_type;

	return js_udp_sendto(sock, clnt_addr, msg_buf, n, udp_struct);
}

int js_udp_sendpacket(int sock, struct sockaddr_in* clnt_addr, int msg_from, int msg_type)
{
	return js_udp_ssendto(sock, clnt_addr, msg_from, msg_type, "0", 1);
}

int js_rudp_sendto(int sock, struct sockaddr_in* clnt_addr, const void* msg_buf, size_t n, js_udp_struct udp_struct)
{
	int i;
	char udp_buf[UDP_BUF_SIZE];
	char buf[UDP_BUF_SIZE];
	u_long block = 0;
	u_long nonBlock = 1;

	if (js_udp_encode(udp_buf, msg_buf, n, udp_struct) == -1)
	{
		puts("\n###\nSend Failed!\n###\n");
		return -1;
	}

	ioctlsocket(sock, FIONBIO, &nonBlock);//논블록으로 바꿀 위치 잘 생각하기1
	for (i = 0; i < 3; i++)//몇회 시도해보고 안되면 릴레이 전환 생각해보기
	{
		sendto(sock, udp_buf, udp_buf[0], 0, (struct sockaddr*)clnt_addr, sizeof(*clnt_addr));
		printf("\n###\nRUDP Send Client\nIP: %s Port: %d\n###\n", inet_ntoa((*clnt_addr).sin_addr), ntohs((*clnt_addr).sin_port));
		Sleep(RUDP_SLEEP);

		if (recv(sock, buf, UDP_BUF_SIZE, 0) > 0)//다른 스레드에서 뺏어감.
		{
			printf("\n###\nWrite OK!\nOrder: %d\n###\n", udp_buf[2]);
			printf("\n###\nReceive OK!\nOrder: %d\n###\n", buf[5]);
			//if ((buf[4] == OK) && (buf[5] == udp_buf[2]) && (buf[1] == udp_buf[1]))//조건 만족이 안되고있음.
			if ((buf[4] == OK) && (buf[5] == udp_buf[2]))//조건 만족이 안되고있음.
			{
				udp_order++;
				printf("\n###\nNext UDP order: %d\n###\n", udp_order);
				if (udp_order > MAX_UDP_ORDER)
					udp_order = 0;

				break;
			}
		}
		puts("\n###\nRetry Send...!\n###\n");
		if(i==3)
			puts("\n###\nRUDP Failed\n###\n");
	}
	ioctlsocket(sock, FIONBIO, &block);//논블록으로 바꿀 위치 잘 생각하기2

	return 0;
}

int js_rudp_ssendto(int sock, struct sockaddr_in* clnt_addr, int msg_clntcnt, int msg_order, int msg_from, int msg_type, const void* msg_buf, size_t n)
{
	js_udp_struct udp_struct;

	udp_struct.msg_clntcnt = msg_clntcnt;
	udp_struct.msg_order = msg_order;
	udp_struct.msg_from = msg_from;
	udp_struct.msg_type = msg_type;

	return js_rudp_sendto(sock, clnt_addr, msg_buf, n, udp_struct);
}

int js_rudp_sendpacket(int sock, struct sockaddr_in* clnt_addr, int msg_clntcnt, int msg_order, int msg_from, int msg_type)
{
	return js_rudp_ssendto(sock, clnt_addr, msg_clntcnt, msg_order, msg_from, msg_type, "0", 1);
}

//나중에 비트단위로 쪼개서 보내기
int js_tcp_encode(char tcp_buf[], const void* msg_buf, size_t n, js_tcp_struct tcp_struct)
{
	tcp_buf[0] = n + 4;//메시지의 길이. 헤더 개수 + 맨 뒤의 null
	tcp_buf[1] = tcp_struct.msg_from;
	tcp_buf[2] = tcp_struct.msg_type;

	memcpy(&tcp_buf[3], msg_buf, n);
	if (n > TCP_BUF_SIZE - 4)
		return -1;

	tcp_buf[n + 3] = '\0';

	return tcp_buf[0];
}

int js_tcp_write(int sock, const void* msg_buf, size_t n, js_tcp_struct tcp_struct)
{
	char tcp_buf[TCP_BUF_SIZE];

	if (js_tcp_encode(tcp_buf, msg_buf, n, tcp_struct) == -1)
		return -1;

	return send(sock, tcp_buf, tcp_buf[0], 0);//n+멤버개수+2
}

int js_tcp_swrite(int sock, int msg_from, int msg_type, const void* msg_buf, size_t n)
{
	js_tcp_struct tcp_struct;

	tcp_struct.msg_from = msg_from;
	tcp_struct.msg_type = msg_type;

	return js_tcp_write(sock, msg_buf, n, tcp_struct);
}

int js_tcp_packet(int sock, int msg_from, int msg_type)
{
	return js_tcp_swrite(sock, msg_from, msg_type, "0", 1);
}

#endif
