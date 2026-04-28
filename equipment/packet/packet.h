#ifndef PACKET_H
#define PACKET_H
#include "common.h"
#include "hsms.h"
#include "sensor_data.h"
#include "secs2.h"
#include "network.h"

// HSMS 패킷 생성 후 전송
int send_sensor_packet(int sock, uint64_t timestamp, SensorData* data, uint16_t count);

#endif