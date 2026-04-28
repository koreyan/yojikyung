#ifndef SENSOR_CLASS_H
#define SENSOR_CLASS_H

#include "common.h"

//------------------------------------------------------------
// 센서 클래스 (정상 범위 정의)
//------------------------------------------------------------
typedef struct {

    uint8_t class_id;       // [Module(4bit)][SensorType(4bit)]
    float normal_min;       // 정상 최소값
    float normal_max;       // 정상 최대값

} SensorClass;
#endif