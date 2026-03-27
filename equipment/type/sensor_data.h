#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "common.h"

//------------------------------------------------------------
// 센서 데이터 구조 (패킷용)
//------------------------------------------------------------
typedef struct {

    uint16_t sensor_id;     // 센서 ID  [Module|SensorType|Index]
    float value;            // 센서 측정값

} SensorData;

#endif