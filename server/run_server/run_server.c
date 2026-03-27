#include "run_server.h"


/* =========================
   클라이언트 처리 루프
========================= */
void run_client_loop(int client_fd)
{
    RecvBuffer rb = { .write_pos = 0 };

    while (1)
    {
        int ret = recv_data(client_fd, &rb);

        if (ret <= 0) {
            printf("Client disconnected\n");
            break;
        }

        process_buffer(&rb);
    }
}


int run(){
    int server_fd = create_server();
    int client_fd = accept_client(server_fd);

    run_client_loop(client_fd);

    close(client_fd);
    close(server_fd);

    return -1;
}