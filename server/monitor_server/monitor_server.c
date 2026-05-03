#include "monitor_server.h"
#include <time.h>
#include "json_builder/json_builder.h"

static int server_fd;
static int clients[MAX_CLIENTS];

// =========================
// 서버 초기화 (non-blocking 포함)
// =========================
void monitor_server_init(int port)
{
    struct sockaddr_in addr;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket (monitor)");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind (monitor)");
        return;
    }
    if (listen(server_fd, 5) < 0) {
        perror("listen (monitor)");
        return;
    }

    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = -1;

    printf("📡 Monitoring server listening on %d\n", port);
}

void monitor_accept_clients()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;  // non-blocking

    int ret = select(server_fd + 1, &readfds, NULL, NULL, &tv);

    if (ret < 0) {
        perror("select");
        return;
    }

    // 👉 연결 요청 없음
    if (ret == 0) {
        return;
    }

    // 👉 accept 가능 상태
    if (FD_ISSET(server_fd, &readfds)) {

        struct sockaddr_in cli;
        socklen_t len = sizeof(cli);

        int new_fd = accept(server_fd, (struct sockaddr*)&cli, &len);

        if (new_fd < 0) {
            // non-blocking 환경에서 흔히 발생
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                return;

            perror("accept");
            return;
        }

        // 👉 클라이언트 저장
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == -1) {
                clients[i] = new_fd;
                printf("✅ Client connected (%d)\n", new_fd);
                return;
            }
        }

        // 👉 자리 없으면 버림
        printf("❌ Client rejected (full)\n");
        close(new_fd);
    }
}




// =========================
// 클라이언트 전송 (안전)
// =========================

void monitor_send_all(const char *msg)
{

    uint32_t msg_len = strlen(msg);
    uint32_t net_len = htonl(msg_len); // 네트워크 바이트 순서

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != -1)
        {
            // =========================
            // 1️⃣ 길이 전송 (4바이트)
            // =========================
            size_t sent = 0;
            while (sent < 4)
            {
                ssize_t n = send(clients[i], ((char*)&net_len) + sent, 4 - sent, 0);

                if (n > 0) sent += n;
                else if (n == 0)
                {
                    printf("❌ Client disconnected (%d)\n", clients[i]);
                    close(clients[i]);
                    clients[i] = -1;
                    break;
                }
                else
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    printf("❌ send length error (%d): %s\n", clients[i], strerror(errno));
                    close(clients[i]);
                    clients[i] = -1;
                    break;
                }
            }

            if (clients[i] == -1) continue;

            // =========================
            // 2️⃣ JSON 본문 전송
            // =========================
            sent = 0;
            while (sent < msg_len)
            {
                ssize_t n = send(clients[i], msg + sent, msg_len - sent, 0);

                if (n > 0) sent += n;
                else if (n == 0)
                {
                    printf("❌ Client disconnected (%d)\n", clients[i]);
                    close(clients[i]);
                    clients[i] = -1;
                    break;
                }
                else
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                    printf("❌ send data error (%d): %s\n", clients[i], strerror(errno));
                    close(clients[i]);
                    clients[i] = -1;
                    break;
                }
            }
        }
    }
}

static long get_now_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void monitor_periodic_task() {
    static long last_send_time = 0;
    long now = get_now_ms();

    if (now - last_send_time >= 1000) {
        char *json = build_periodic_json();
        if (json) {
            monitor_send_all(json);
        }
        last_send_time = now;
    }
}