#ifndef PROCESSOR_SENSOR_H
#define PROCESSOR_SENSOR_H

#include "sensordata.h"
#include "anomaly_detection.h"
#include "moving_average.h"

void process_sensor(SensorData *data);

#endif