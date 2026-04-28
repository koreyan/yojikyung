#ifndef DECODER_H
#define DECODER_H

#include "hsms_header_parser.h"
#include "secs2_parser.h"
#include "packet_data.h"

/* 전체 패킷 디코딩 */
void decode_packet(uint8_t *packet, int len, PacketData *pkt);



#endif