#ifndef PACKET_DATA_H
#define PACKET_DATA_H


#include "common.h"
#include "sensordata.h"

#define MAX_SENSORS 256


/* =========================
   Packet Data
========================= */
typedef struct {
    uint64_t timestamp;
    uint16_t count;
    SensorData sensors[MAX_SENSORS];
} PacketData;


#endif

