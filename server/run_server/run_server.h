#ifndef RUN_SERVER_H
#define RUN_SERVER_H

#include "controller.h"


/* =========================
   클라이언트 처리 루프
========================= */
void run_client_loop(int client_fd);
int run();

#endif