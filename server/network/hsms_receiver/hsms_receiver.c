#include "hsms_receiver.h"

/* =========================
   RecvBuffer 관리
========================= */

void buffer_append(RecvBuffer *rb, uint8_t *data, int len)
{
    if (rb->write_pos + len > BUFFER_SIZE) {
        printf("Buffer overflow\n");
        exit(1);
    }

    memcpy(rb->buffer + rb->write_pos, data, len);
    rb->write_pos += len;
}

/* =========================
   소켓에서 데이터 수신
========================= */

int recv_data(int client_fd, RecvBuffer *rb)
{
    uint8_t tmp[1024];  // 임시 수신 버퍼

    // 소켓에서 데이터 읽기
    int n = recv(client_fd, tmp, sizeof(tmp), 0);

    // 연결 종료 또는 에러
    if (n <= 0)
        return n;

    // RecvBuffer에 누적
    buffer_append(rb, tmp, n);

    return n;
}


/* =========================
   패킷 추출 (핵심)
========================= */

int try_extract_packet(RecvBuffer *rb, uint8_t *out_packet, int *out_len)
{
    if (rb->write_pos < HSMS_HEADER_SIZE)
        return 0;

    uint32_t body_len;
    memcpy(&body_len, rb->buffer, 4);
    body_len = ntohl(body_len);

    int total_len = HSMS_HEADER_SIZE + body_len;

    if (rb->write_pos < total_len)
        return 0;

    // 패킷 복사
    memcpy(out_packet, rb->buffer, total_len);
    *out_len = total_len;

    // 버퍼 shift
    memmove(rb->buffer, rb->buffer + total_len, rb->write_pos - total_len);
    rb->write_pos -= total_len;

    return 1;
}


