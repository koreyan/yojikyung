#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

// .env 파일에서 설정값을 읽어오는 함수
static void load_env(char *host, int *port) {
    // 기본값 설정
    strcpy(host, SERVER_IP);
    *port = PORT;

    FILE *fp = fopen(".env", "r");
    if (fp == NULL) {
        fp = fopen("run/.env", "r");
        if (fp == NULL) {
            fp = fopen("equipment/run/.env", "r");
            if (fp == NULL) {
                fp = fopen("../run/.env", "r");
                if (fp == NULL) return; 
            }
        }
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "SERVER_HOST=", 12) == 0) {
            sscanf(line + 12, "%s", host);
        } else if (strncmp(line, "EQUIPMENT_PORT=", 15) == 0) {
            *port = atoi(line + 15);
        }
    }
    fclose(fp);
}


// 서버와 TCP 연결을 생성하고 연결된 socket fd를 반환하는 함수
int connect_server(){
    printf("connecting...");
    // socket 생성
    // AF_INET : IPv4 주소 체계 사용
    // SOCK_STREAM : TCP 프로토콜 사용
    // 0 : 기본 프로토콜 선택 (TCP)
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0){
        perror("socket failed");
        exit(1);
    }
    
    // 서버 주소 정보를 저장할 구조체
    struct sockaddr_in server;
    char host[256];
    int port;

    // .env 파일로부터 서버 정보 로드
    load_env(host, &port);
    printf("\n🚀 Connecting to Server [%s:%d]...\n", host, port);

    // 사용할 주소 체계 설정 (IPv4)
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // 문자열 형태의 IP 주소를 binary 형태로 변환
    if (inet_pton(AF_INET, host, &server.sin_addr) <= 0) {
        // IP가 아닌 도메인 이름인 경우 처리
        struct hostent *he = gethostbyname(host);
        if (he == NULL) {
            fprintf(stderr, "❌ DNS Resolution Failed: %s\n", host);
            exit(1);
        }
        memcpy(&server.sin_addr, he->h_addr_list[0], he->h_length);
    }

    // 서버에 연결 요청
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror("❌ Connection failed");
        exit(1);
    }

    // 연결 성공 메시지 출력
    printf("✅ Connected to server successfully!\n\n");
    return sock;
}
