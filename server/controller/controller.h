#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"
#include "hsms_receiver.h"
#include "decoder.h"
#include "packet_data.h"
#include "process_sensor.h"
#include "debug.h"

/* =========================
   패킷 처리 (Hook)
========================= */
void handle_packet(uint8_t *packet, int len);


/* =========================
   버퍼 처리
========================= */
void process_buffer(RecvBuffer *rb);


#endif 