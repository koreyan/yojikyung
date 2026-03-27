#ifndef DEBUG_HANDLER
#define DEBUG_HANDLER

#include "common.h"
#include "sensordata.h"
#include "sensor_id_util.h"

void print_hex(uint8_t *data, int len);
void print_sensor(SensorData *s, int idx);

#endif 