#define _CRT_SECURE_NO_WARNINGS
#include "jsmanager.h"

void js_client();

int main(void)
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		error_handling("WSAStartup() error!");

	udp_order_Mutex = CreateMutex(NULL, FALSE, NULL);

	js_set_sock();

	js_client();

	WSACleanup();

	return 0;
}

void js_client()
{
	int strLen;
	char buf[TCP_BUF_SIZE];

	while (1)
	{
		strLen = recv(hSocket, buf, TCP_BUF_SIZE, 0);//iocp로 이후 변환고려하기

		if (strLen == -1)
		{
			error_handling("read() error!");
			printf("Error : %s\n", strerror(errno));
		}

		printf("Received Tcp Packet: %d", strLen);
		js_tcp_read(strLen, buf);
	}

	closesocket(hSocket); closesocket(uSocket);
}