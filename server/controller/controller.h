#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "common.h"
#include "hsms_receiver.h"
#include "decoder.h"
#include "packet_data.h"
#include "monitor_server.h"
#include "json_builder.h"
#include "debug.h"
#include "../storage/history_storage.h"
#include "../storage/file_logger.h"

/* =========================
   패킷 처리 (Hook)
========================= */
void handle_packet(uint8_t *packet);


/* =========================
   버퍼 처리
========================= */
void process_buffer(RecvBuffer *rb);


#endif 