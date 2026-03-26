#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"

#define SERVER_IP "127.0.0.1"
#define PORT 12345

// 서버와 TCP 연결을 생성하고 연결된 socket fd를 반환하는 함수
int connect_server();

#endif