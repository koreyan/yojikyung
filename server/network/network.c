#include "network.h"


/* =========================
   서버 생성
========================= */
int create_server()
{
    int server_fd;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);
    return server_fd;
}

/* =========================
   클라이언트 연결
========================= */
int accept_client(int server_fd)
{
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int client_fd = accept(server_fd,
                           (struct sockaddr*)&client_addr,
                           &len);

    if (client_fd < 0) {
        if (errno == EINTR) {
            printf("⚠️ accept interrupted by signal\n");
        } else if (errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("accept");
        }
        return -1;
    }

    printf("Client connected\n");
    return client_fd;
}


