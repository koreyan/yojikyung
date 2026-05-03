#ifndef SENSOR_STATE_H
#define SENSOR_STATE_H

#include "common.h"

#define MAX_SENSOR 1024
#define WINDOW_SIZE 5

typedef struct {
    uint16_t sensor_id;

    float window[WINDOW_SIZE];
    int index;
    int count;

    float sum;   // O(1) 이동평균용

    // 주기적 모니터링을 위한 상태
    float last_avg;
    int has_anomaly;
    float anomaly_value;
} SensorState;

SensorState* get_sensor_state(uint16_t sensor_id);
void get_all_sensor_states(SensorState** out_states, int* out_count);

#endif