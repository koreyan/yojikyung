#ifndef PACKET_H
#define PACKET_H
#include "packet_protocol.h"
#include "sensor_protocol.h"

#define MAX_PACKET_SIZE 4096
uint8_t packet_buffer[MAX_PACKET_SIZE];

// buffer: 결과 패킷이 저장될 메모리
// data: 센서 데이터 배열
// count: 센서 개수
// now: timestamp
// return: 패킷 전체 크기 (bytes)
uint32_t build_packet(uint8_t* buffer, SensorData data[], int count, uint64_t now);


// 패킷 전체 전송 함수
void send_packet(int sock, uint8_t* buffer, uint32_t size);
#endif