#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include "sensor_state.h"


float update_moving_average(SensorState *s, float new_value);
#endif