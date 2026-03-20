#ifndef SENSOR_H
#define SENSOR_H
#include "sensor_protocol.h"

// class_id로 SensorClass 검색
SensorClass* find_sensor_class(uint8_t class_id);

static void add_sensor(
    Sensor sensors[],
    int *i,
    Module module,
    SensorType type,
    uint8_t index,
    int period);

// 센서 초기화 하는 함수
int init_sensors(Sensor sensors[]);

// 센서 생성 값 생성 함수 
float generate_sensor_value(uint16_t sensor_id);

// 패킷에 붙일 센서 데이터 값 생성하는 함수 
int generate_sensor_packet(Sensor sensors[], int sensor_count, SensorData packet_data[], int max_count, uint64_t now);

#endif 