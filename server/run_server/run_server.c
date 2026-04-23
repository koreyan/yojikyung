#include "run_server.h"


/* =========================
   클라이언트 처리 루프
========================= */
void run_client_loop(int client_fd)
{
    RecvBuffer rb = { .write_pos = 0 };

    while (1)
    {
        int ret = recv_data(client_fd, &rb); // 장비로부터 데이터를 받음

        if (ret <= 0) {
            printf("Client disconnected\n");
            break;
        }
        monitor_accept_clients();   // 모니터링 클라이언트 연결 처리
        process_buffer(&rb);
    }
}


int run(){
    
    init_storage(); // 서버 시작 시 저장소 초기화
    init_file_logger("sensor_data.log");  // 디스크 로거 시작
    
    int server_fd = create_server(); // 장비용 서버 열기
    monitor_server_init(9000);  // 모니터링용 서버 열기

    int client_fd = accept_client(server_fd); // 장비 클라이언트가 들어올 때까지 대기 

    run_client_loop(client_fd);

    close(client_fd);
    close(server_fd);

    close_file_logger(); // 종료 시 파일 닫기
    cleanup_storage();   // 종료 시 메모리 해제
    return 0;
}