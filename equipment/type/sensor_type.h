#ifndef SENSOR_TYPE_H
#define SENSOR_TYPE_H

#include "common.h"

//------------------------------------------------------------
// 센서 인스턴스 관리 구조
//------------------------------------------------------------
typedef struct {

    uint16_t sensor_id;     // [Module|SensorType|Index]

    uint32_t period_ms;     // 데이터 생성 주기
    uint64_t next_time_ms;  // 다음 데이터 생성 시각

} Sensor;

#endif