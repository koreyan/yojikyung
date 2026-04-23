#include "controller.h"


/* =========================
   패킷 처리 (Hook)
========================= */
void handle_packet(uint8_t *packet) {
    uint32_t body_len;
    // HSMS Header (4바이트 length) 추출
    memcpy(&body_len, packet, 4);
    body_len = ntohl(body_len);

    PacketData pkt;
    memset(&pkt, 0, sizeof(PacketData));

    // 실제 데이터 영역(Header 14 + Body)만 디코더에 전달 (뒷부분 0 무시)
    decode_packet(packet, 14 + body_len, &pkt);

    if (pkt.count > 0) {
        save_to_storage(&pkt);
        add_to_log_buffer(&pkt); // 3번 문제 관련: 로그 버퍼 적재
        
        char *json = build_json(&pkt);
        if (json) monitor_send_all(json);
    }
}
/* =========================
   버퍼 처리
========================= */

// 이 함수가 메인 루프에서 호출되어 '조립'을 수행합니다.
void process_buffer(RecvBuffer *rb) {
    uint8_t packet_unit[4096];

    while (try_extract_packet(rb, packet_unit)) {
        handle_packet(packet_unit);
    }
}