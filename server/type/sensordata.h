#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include "common.h"

/* =========================
   Sensor Data
========================= */
typedef struct {
    uint16_t sensor_id;
    float value;
} SensorData;

#endif