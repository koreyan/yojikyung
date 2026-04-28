#ifndef HSMS_HEADER_H
#define HSMS_HEADER_H

#include "common.h"
#include "hsms_header.h"

/* HSMS 헤더를 파싱하는 함수 */
void parse_hsms_header(uint8_t *buf, HSMSHeader *h);

#endif