#include "hsms_receiver.h"


#define FIXED_SIZE 4096

// 소켓에서 들어오는 조각 데이터를 버퍼 끝에 추가
void buffer_append(RecvBuffer *rb, uint8_t *data, int len) {
    if (rb->write_pos + len <= BUFFER_SIZE) {
        memcpy(rb->buffer + rb->write_pos, data, len);
        rb->write_pos += len;
    }
}

// 데이터 수신 루틴
int recv_data(int client_fd, RecvBuffer *rb) {
    uint8_t tmp[2048];
    int n = recv(client_fd, tmp, sizeof(tmp), 0);
    if (n > 0) {
        buffer_append(rb, tmp, n);
    }
    return n;
}

// 4096바이트가 차 있으면 꺼내주고, 남은 건 앞으로 당김 (조립 지원)
int try_extract_packet(RecvBuffer *rb, uint8_t *out_packet) {
    if (rb->write_pos < FIXED_SIZE) {
        return 0; // 아직 데이터 부족
    }

    // 4096바이트 복사
    memcpy(out_packet, rb->buffer, FIXED_SIZE);

    // 잔여 데이터 처리 (Memmove로 앞으로 당김)
    int remaining = rb->write_pos - FIXED_SIZE;
    if (remaining > 0) {
        memmove(rb->buffer, rb->buffer + FIXED_SIZE, remaining);
    }
    rb->write_pos = remaining;

    return 1;
}