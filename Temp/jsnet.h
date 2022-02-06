#ifndef JSNET
#define JSNET
#define _CRT_SECURE_NO_WARNINGS

//서버 소켓 관련 설정
#define G_SERV_IP "59.2.102.80"
#define SERV_PORT 2323
#define HOST_PORT 2424
#define CLIENT_PORT 2525

#define TCP_BUF_SIZE 1000
#define UDP_BUF_SIZE 1000
#define MAX_UDP_ORDER 1000

#define MAX_CLIENT 8
#define NAME_SIZE 20

//메시지 처리 매크로
//클라이언트 프로그램도 동일해야 하는 매크로.
//msg_from
#define SERVER 0
#define HOST 1
#define CLIENT 2

//msg_type
#define OK 0
#define MESSAGE 1
#define CLNTCNT 2
#define BEHOST 3
#define RCVADR 4
#define HSTUN 5
#define CSTUN 6

SOCKET hSocket, uSocket, hostSock;
struct sockaddr_in serv_addr;

int clntcnt;
HANDLE udp_order_Mutex;
int udp_order;
int isHStun;
int isCStun;

SOCKADDR_IN host_addr, host_clnt_addr, clnt_clnt_addr;
int p2pcnt;
SOCKADDR_IN p2p_addr[MAX_CLIENT];
char client_name[MAX_CLIENT][NAME_SIZE];
int p2p_order;
int p2p_orders[MAX_CLIENT];

typedef struct {
	int msg_from;
	int msg_type;
} js_tcp_struct;

typedef struct {
	int msg_clntcnt;
	int msg_order;
	int msg_from;
	int msg_type;
} js_udp_struct;//체크섬 공부해서 무결성 보장하기/도착을 보장하는 함수 만들기

#endif
