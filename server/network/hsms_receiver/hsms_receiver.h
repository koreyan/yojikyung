#ifndef HSMS_RECEIVER_H
#define HSMS_RECEIVER_H

#include "common.h"
#include "network.h"

#define BUFFER_SIZE 8192
#define HSMS_HEADER_SIZE 14


typedef struct {
    uint8_t buffer[BUFFER_SIZE];
    int write_pos;
} RecvBuffer;


/* =========================
   RecvBuffer 관리
========================= */
void buffer_append(RecvBuffer *rb, uint8_t *data, int len);


/* =========================
   소켓에서 데이터 수신
========================= */
int recv_data(int client_fd, RecvBuffer *rb);

/* =========================
   패킷 추출 (핵심)
========================= */
int try_extract_packet(RecvBuffer *rb, uint8_t *out_packet, int *out_len);


#endif