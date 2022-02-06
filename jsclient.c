#define _CRT_SECURE_NO_WARNINGS
#include "jsmanager.h"

void js_client();

int main(void)
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		error_handling("WSAStartup() error!");

	js_set_sock();

	js_client();

	WSACleanup();

	return 0;
}

unsigned WINAPI TaskRec(void* arg)
{
	SOCKADDR_IN remoteAddr;
	int	remoteAddrLen = sizeof(remoteAddr);
	int iResult;
	char buf[UDP_BUF_SIZE];

	while (1)
	{
		iResult = recvfrom(uSocket, buf, UDP_BUF_SIZE, 0, (struct sockaddr*)&remoteAddr, &remoteAddrLen);

		puts("Recv Message!");

		if (iResult > 0) {
			printf("\n###\nPeer-to-peer Message\nIP: %s Port: %d\nmessage: %s\n###\n", inet_ntoa(remoteAddr.sin_addr), ntohs(remoteAddr.sin_port), buf);
		}
		else {
			puts("Error: Peer closed.");
		}
	}

	return;
}

void js_client()
{
	char ID[2];
	SOCKADDR_IN peer_addr;
	int strLen;
	char buf[UDP_BUF_SIZE];

	printf("Identificationnumber: ");
	scanf("%c", &ID[0]);
	ID[1] = '\0';

	sendto(uSocket, ID, strlen(ID), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	while (1) {
		SOCKADDR_IN remoteAddr;
		int	remoteAddrLen = sizeof(remoteAddr);

		int iResult = recvfrom(uSocket, buf, UDP_BUF_SIZE, 0, (struct sockaddr*)&remoteAddr, &remoteAddrLen);

		if (iResult > 0) {
			memcpy(&peer_addr, buf, sizeof(SOCKADDR_IN));

			printf("\n###\nPeer-to-peer Endpoint\nIP: %s Port: %d\n###\n", inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));

			break;
		}
		else
			printf("%d", WSAGetLastError());
	}

	_beginthreadex(NULL, 0, TaskRec, NULL, 0, NULL);

	while (1)
	{
		puts("Send Message!");

		sendto(uSocket, "Hello world!", strlen("Hello world!"), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));

		Sleep(500);
	}

	getchar();

	closesocket(uSocket); closesocket(uSocket);
}