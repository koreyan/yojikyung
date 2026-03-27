#include "hsms_header_parser.h"



/* HSMS 헤더를 파싱하는 함수 */
void parse_hsms_header(uint8_t *buf, HSMSHeader *h)
{
    // network byte order → host 변환
    h->length       = ntohl(*(uint32_t*)(buf));
    h->session_id   = ntohs(*(uint16_t*)(buf + 4));

    // 단일 바이트는 그대로 사용
    h->stream       = buf[6];
    h->function     = buf[7];
    h->p_type       = buf[8];
    h->s_type       = buf[9];

    h->system_bytes = ntohl(*(uint32_t*)(buf + 10));
}