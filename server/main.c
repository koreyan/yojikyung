#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define BUFFER_SIZE 4096

//------------------------------------------------------------
// 패킷 구조 (클라이언트와 동일해야 함)
//------------------------------------------------------------
typedef struct {
    uint32_t size;
    uint64_t timestamp;
    uint16_t sensor_count;
} PacketHeader;

typedef struct {
    uint16_t sensor_id;
    float value;
} SensorData;


//------------------------------------------------------------
// 전체 데이터 수신 함수 (TCP는 나눠서 올 수 있음)
//------------------------------------------------------------
int recv_all(int sock, void *buffer, int size)
{
    int total = 0;
    int bytes;

    while(total < size)
    {
        bytes = recv(sock, (char*)buffer + total, size - total, 0);
        if(bytes <= 0)
            return -1;

        total += bytes;
    }

    return total;
}


//------------------------------------------------------------
// 메인 서버
//------------------------------------------------------------
int main()
{
    int server_fd, client_sock;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // 1. 소켓 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("bind failed");
        exit(1);
    }

    if(listen(server_fd, 5) < 0)
    {
        perror("listen failed");
        exit(1);
    }

    printf("Server listening on port %d...\n", PORT);

    // 4. accept
    client_sock = accept(server_fd, (struct sockaddr*)&addr, &addr_len);
    printf("Client connected!\n");

    while(1)
    {
        PacketHeader header;

        // 5. 헤더 수신
        if(recv_all(client_sock, &header, sizeof(header)) <= 0)
        {
            printf("Client disconnected\n");
            break;
        }

        printf("\n[Packet] time=%llu, count=%d\n",
               header.timestamp,
               header.sensor_count);

        // 6. 센서 데이터 수신
        int data_size = header.sensor_count * sizeof(SensorData);
        SensorData *data = malloc(data_size);

        if(recv_all(client_sock, data, data_size) <= 0)
        {
            free(data);
            break;
        }

        // 7. 출력
        for(int i = 0; i < header.sensor_count; i++)
        {
            uint16_t id = data[i].sensor_id;

            uint8_t module = (id >> 12) & 0x0F;
            uint8_t type   = (id >> 8)  & 0x0F;
            uint8_t index  = id & 0xFF;

            printf("  ID=0x%X%X%02X (M=%d T=%d I=%d), Value=%.2f\n",
                module, type, index,
                module, type, index,
                data[i].value);
            }

        free(data);
    }

    close(client_sock);
    close(server_fd);

    return 0;
}