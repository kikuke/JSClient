#ifndef JSSETTING
#define JSSETTING
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include "jsnet.h"
#include "jscontrol.h"

void js_set_sock()
{
	SOCKADDR_IN clnt_addr;
	puts("\n#####\nSet Socket...\n#####\n");

	uSocket = socket(PF_INET, SOCK_DGRAM, 0);
	if (uSocket == INVALID_SOCKET)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(G_SERV_IP);
	serv_addr.sin_port = htons(SERV_PORT);

	/*
	memset(&clnt_addr, 0, sizeof(clnt_addr));
	clnt_addr.sin_family = AF_INET;
	clnt_addr.sin_addr.s_addr = INADDR_ANY;
	clnt_addr.sin_port = CLIENT_PORT;

	if (bind(uSocket, (LPSOCKADDR)&clnt_addr, sizeof(clnt_addr)) == SOCKET_ERROR)
		error_handling("bind() error");
		*/

	return;
}

#endif
