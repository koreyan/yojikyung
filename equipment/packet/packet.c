#include "packet.h"

static uint32_t g_sys = 1;

// HSMS Header 생성
void build_hsms_header(HSMSHeader* h,
                       uint16_t session_id,
                       uint8_t stream,
                       uint8_t function,
                       uint8_t s_type,
                       uint32_t body_len)
{
    h->length = htonl(body_len);     // body 길이
    h->session_id = htons(session_id);

    h->stream = stream;              // SxFy
    h->function = function;

    h->p_type = 0;                   // SECS-II
    h->s_type = s_type;              // data = 0

    h->system_bytes = htonl(g_sys++);
}


// S6F1 구조: timestamp + count + sensor list 생성
uint32_t build_secs_body(uint8_t* buffer,
                         uint64_t timestamp,
                         SensorData* data,
                         uint16_t count)
{
    uint8_t* p = buffer;

    p = write_list(p, 3);         // [timestamp, count, data list]

    p = write_u8(p, timestamp);  // timestamp
    p = write_u2(p, count);      // sensor 개수

    p = write_list(p, count);    // sensor list

    for (int i = 0; i < count; i++) {
        p = write_list(p, 2);    // [sensor_id, value]

        p = write_u2(p, data[i].sensor_id);
        p = write_f4(p, data[i].value);
    }

    return (uint32_t)(p - buffer);
}

// HSMS 패킷 생성 후 전송
int send_sensor_packet(int sock,
                       uint64_t timestamp,
                       SensorData* data,
                       uint16_t count)
{
    uint8_t body[4096];
    uint8_t packet[4096];

    // 1. SECS-II Body 생성
    uint32_t body_len = build_secs_body(body, timestamp, data, count);

    // 2. HSMS Header 생성
    HSMSHeader header;
    build_hsms_header(&header,
                      0,      // session_id
                      6,      // S6
                      1,      // F1
                      0,      // data message
                      body_len);

    // 3. Header + Body 결합
    memcpy(packet, &header, sizeof(header));
    memcpy(packet + sizeof(header), body, body_len);

    uint32_t total_size = sizeof(header) + body_len;

    // 4. 전송
    return send(sock, packet, total_size, 0);
}




// // 패킷 생성 함수
// uint32_t build_packet(uint8_t* buffer, SensorData data[], int count, uint64_t now)
// {
//     PacketHeader header;

//     // 헤더 구성
//     header.timestamp = now;
//     header.sensor_count = count;
//     header.size = sizeof(PacketHeader)
//                 + count * sizeof(SensorData);

//     // 1. 헤더 복사
//     memcpy(buffer, &header, sizeof(PacketHeader));

//     // 2. 센서 데이터 복사
//     memcpy(buffer + sizeof(PacketHeader),
//            data,
//            count * sizeof(SensorData));

//     return header.size;
// }



// // 패킷 전체 전송 함수
// void send_packet(int sock, uint8_t* buffer, uint32_t size)
// {
//     uint32_t total_sent = 0;

//     while(total_sent < size)
//     {
//         int sent = send(sock,
//                         buffer + total_sent,
//                         size - total_sent,
//                         0);

//         if(sent <= 0)
//         {
//             // 에러 처리 필요
//             perror("send failed");
//             return;
//         }

//         total_sent += sent;
//     }
// }