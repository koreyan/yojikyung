#include "packet.h"

#define FIXED_SIZE 4096
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

// HSMS 패킷 생성 후 전송 (고정패킷방식으로 변경)
int send_sensor_packet(int sock,
                       uint64_t timestamp,
                       SensorData* data,
                       uint16_t count)
{
    // [수정사항] 스택 메모리 낭비를 줄이기 위해 버퍼 하나로 통합 관리 가능하지만, 
    // 기존 구조를 유지하며 안전하게 0으로 초기화합니다.
    uint8_t body[FIXED_SIZE];
    uint8_t packet[FIXED_SIZE];

    // [수정사항] 전송 버퍼 전체를 0으로 초기화 (나머지 공간은 패딩 처리됨)
    memset(packet, 0, FIXED_SIZE);
    memset(body, 0, FIXED_SIZE);

    // 1. SECS-II Body 생성
    uint32_t body_len = build_secs_body(body, timestamp, data, count);

    // 2. HSMS Header 생성
    HSMSHeader header;
    build_hsms_header(&header, 0, 6, 1, 0, body_len);

    // 3. Header + Body 결합
    memcpy(packet, &header, sizeof(header));
    memcpy(packet + sizeof(header), body, body_len);

    // [수정사항] total_size 변수는 실제 데이터 크기 확인용으로만 사용
    // uint32_t actual_data_size = sizeof(header) + body_len;

    // [수정사항] 기존 total_size 대신 무조건 FIXED_SIZE(4096)를 보냄
    // 이로써 서버는 항상 4096바이트씩 끊어 읽을 수 있는 '경계'를 갖게 됩니다.
    return send(sock, packet, FIXED_SIZE, 0);
}