#include "network.h"


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

    // 사용할 주소 체계 설정 (IPv4)
    server.sin_family = AF_INET;

    // 포트 번호 설정
    // htons() : host byte order -> network byte order 변환
    // 네트워크에서는 big-endian 형식을 사용하기 때문에 변환 필요
    server.sin_port = htons(PORT);

    // 문자열 형태의 IP 주소를 binary 형태로 변환
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    // 서버에 연결 요청
    // sock : 사용할 소켓
    // (struct sokaddr*): sockaddr_in을 일반 sockaddr로 캐스팅
    // sizof(server) : 주소 구조체 크기
    if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0){
        perror("connect failed");
        exit(1);
    }

    // 연결 성공 메시지 출력
    printf("Connected to server\n");
    return sock;
}
