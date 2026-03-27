#ifndef SENSOR_ID_UTIL_H
#define SENSOR_ID_UTIL_H

#include "sensor_class.h"
#include "common.h"

#define MODULE_SHIFT 12
#define TYPE_SHIFT 8

#define MAKE_CLASS_ID(module, type)  (((module) << 4) | (type))
#define MAKE_SENSOR_ID(m,t,i)(((m & 0x0F) << 12) | ((t & 0x0F) << 8) | (i & 0xFF))
#define GET_CLASS_ID(sensor_id) ((uint8_t)((sensor_id) >> 8))

// sensor_id → class_id 추출 후 해당 SensorClass 찾기
// → 센서마다 정상 범위(min/max)를 얻기 위해 필요
SensorClass* find_sensor_class(uint16_t sensor_id);

void decode_sensor_id(uint16_t id, int *module, int *type, int *index);
#endif