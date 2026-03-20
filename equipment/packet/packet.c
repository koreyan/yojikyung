#include "packet.h"


// 패킷 생성 함수
uint32_t build_packet(uint8_t* buffer, SensorData data[], int count, uint64_t now)
{
    PacketHeader header;

    // 헤더 구성
    header.timestamp = now;
    header.sensor_count = count;
    header.size = sizeof(PacketHeader)
                + count * sizeof(SensorData);

    // 1. 헤더 복사
    memcpy(buffer, &header, sizeof(PacketHeader));

    // 2. 센서 데이터 복사
    memcpy(buffer + sizeof(PacketHeader),
           data,
           count * sizeof(SensorData));

    return header.size;
}



// 패킷 전체 전송 함수
void send_packet(int sock, uint8_t* buffer, uint32_t size)
{
    uint32_t total_sent = 0;

    while(total_sent < size)
    {
        int sent = send(sock,
                        buffer + total_sent,
                        size - total_sent,
                        0);

        if(sent <= 0)
        {
            // 에러 처리 필요
            perror("send failed");
            return;
        }

        total_sent += sent;
    }
}