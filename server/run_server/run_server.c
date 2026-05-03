#include "run_server.h"
#include <signal.h>

/* =========================
   Graceful Shutdown
========================= */
static volatile int g_running = 1;

static void sigint_handler(int sig) {
    (void)sig;
    printf("\n🛑 Shutdown signal received. Cleaning up...\n");
    g_running = 0;
}


/* =========================
   클라이언트 처리 루프
========================= */
void run_client_loop(int client_fd)
{
    RecvBuffer rb = { .write_pos = 0 };

    // 장비 소켓을 비차단(non-blocking) 모드로 설정
    int flags = fcntl(client_fd, F_GETFL, 0);
    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

    while (g_running)
    {
        int ret = recv_data(client_fd, &rb); // 장비로부터 데이터를 받음

        if (ret < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // 데이터가 아직 없음 -> 모니터링 클라이언트 수락 시도
                monitor_accept_clients();
                usleep(10000); // 10ms 대기 (CPU 점유율 방지)
                continue;
            }
            printf("❌ Equipment connection error\n");
            break;
        } else if (ret == 0) {
            printf("🔄 Equipment disconnected\n");
            break;
        }

        monitor_accept_clients();   // 데이터 수신 후에도 수락 시도
        monitor_periodic_task();    // 1초 주기 전송 체크
        process_buffer(&rb);
    }
}


int run(){
    // SIGPIPE 무시: 끊긴 소켓에 send() 시 프로세스 사망 방지 (Ubuntu 필수)
    signal(SIGPIPE, SIG_IGN);

    // Graceful Shutdown: Ctrl+C / kill 시 안전한 종료
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);

    init_storage(); // 서버 시작 시 저장소 초기화
    init_file_logger("sensor_data.log");  // 디스크 로거 시작
    
    int server_fd = create_server(); // 장비용 서버 열기
    
    // 장비 서버 소켓을 비차단 모드로 설정
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    monitor_server_init(9001);  // 모니터링용 서버 열기 (Bridge와 통신)

    // 장비 재연결 대기 루프
    while (g_running) {
        int client_fd = accept_client(server_fd);

        if (client_fd < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                monitor_accept_clients(); // 장비 연결 대기 중에도 모니터링 수락
                monitor_periodic_task();  // 장비 대기 중에도 1초 주기 전송 체크
                usleep(100000); // 100ms 대기
                continue;
            }
            // 진짜 에러인 경우에만 출력
            if (g_running) perror("accept");
            break;
        }

        run_client_loop(client_fd);
        close(client_fd);
        printf("🔄 Equipment disconnected. Restarting accept loop...\n");
    }

    // Graceful Shutdown: 리소스 정리
    printf("🧹 Cleaning up resources...\n");
    close(server_fd);
    close_file_logger(); // 종료 시 파일 닫기
    cleanup_storage();   // 종료 시 메모리 해제
    printf("✅ Server shutdown complete.\n");
    return 0;
}