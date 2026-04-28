#ifndef MONITOR_SERVER_H
#define MONITOR_SERVER_H

#include "common.h"
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CLIENTS 16

void monitor_server_init(int port);
void monitor_accept_clients();
void monitor_send_all(const char *msg);

#endif