#ifndef SENSOR_STATE
#define SENSOR_STATE

#include "common.h"

// 센서의 "현재 값 상태"를 유지하기 위한 구조체
// → 매번 랜덤이 아니라 이전 값을 기반으로 변화시키기 위해 필요
typedef struct {
    uint16_t sensor_id;
    float value;          // 현재 센서 값 (state)
} SensorState;

#endif