#include "decoder.h"


/* 전체 패킷 디코딩 */
void decode_packet(uint8_t *packet, int len, PacketData *pkt)
{
    HSMSHeader h;

    // 1. HSMS 헤더 파싱
    parse_hsms_header(packet, &h);

    // 2. SECS-II Body 파싱
    parse_secs_body(packet + 14, pkt);
}