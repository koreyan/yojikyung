#ifndef SECS2_PARSER_H
#define SECS2_PARSER_H
#include "common.h"
#include "packet_data.h"

/* SECS-II 타입 상위 6bit */
#define GET_FORMAT(byte) ((byte) & 0xFC)

/* length byte 수 (하위 2bit) */
#define GET_LEN_BYTES(byte) ((byte) & 0x03)

#define TYPE_LIST 0x00
#define TYPE_U2   0xA8
#define TYPE_U8   0xA0
#define TYPE_F4   0x90

/* SECS-II Body 파싱 */
int parse_secs_body(uint8_t *buf, PacketData *pkt);

#endif