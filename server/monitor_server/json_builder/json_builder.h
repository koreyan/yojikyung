#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H
#include <stdio.h>
#include <string.h>

#include "sensor_state.h"
#include "sensor_id_util.h"
#include "packet_data.h"
#include "anomaly_detection.h"
#include "moving_average.h"


#define JSON_BUF_SIZE 32768
#define MAX_SENSORS 128
char* build_json(PacketData *pkt);
char* build_periodic_json();

#endif