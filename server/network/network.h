#ifndef NETWORK_H
#define NETWORK_H

#include "common.h"
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 12345

/* =========================
   서버 생성
========================= */
int create_server();

/* =========================
   클라이언트 연결
========================= */
int accept_client(int server_fd);

#endif