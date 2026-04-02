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
        printf("BEFORE ACCEPT\n");
        monitor_accept_clients();   // 클라이언트 연결 처리
        printf("AFTER ACCEPT\n");
        process_buffer(&rb);
    }
}


int run(){
    int server_fd = create_server(); // 장비용 서버 열기
    monitor_server_init(9000);  // 모니터링용 서버 열기

    int client_fd = accept_client(server_fd);

    run_client_loop(client_fd);

    close(client_fd);
    close(server_fd);

    return -1;
}