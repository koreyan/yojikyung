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
} SensorState;

SensorState* get_sensor_state(uint16_t sensor_id);

#endif