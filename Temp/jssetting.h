#ifndef JSSETTING
#define JSSETTING
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include "jsnet.h"
#include "jscontrol.h"

void js_set_sock()
{
	puts("\n#####\nSet Socket...\n#####\n");

	hSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(G_SERV_IP);
	serv_addr.sin_port = htons(SERV_PORT);

	if (connect(hSocket, (SOCKADDR*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		error_handling("connect() error!");

	puts("\n#####\nConnecting Server!\n#####\n");

	return;
}

#endif
