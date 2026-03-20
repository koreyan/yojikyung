#ifndef PACKET_PROTOCOL_H
#define PACKET_PROTOCOL_H
#include "common.h"

//------------------------------------------------------------
// 패킷 헤더
//------------------------------------------------------------
typedef struct {

    uint32_t size;          // 전체 패킷 크기
    uint64_t timestamp;     // 패킷 생성 시간 (ms)
    uint16_t sensor_count;  // 이번 패킷에 포함된 센서 개수

} PacketHeader;

#endif