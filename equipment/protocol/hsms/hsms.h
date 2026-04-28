#ifndef HSMS_H
#define HSMS_H

#include "common.h"

#pragma pack(push, 1)
typedef struct {
    uint32_t length;        // SECS-II body 전체 길이 (header 제외)
    uint16_t session_id;    // HSMS 세션 식별자

    uint8_t  stream;        // SECS Stream 번호 (SxFy의 S)
    uint8_t  function;      // SECS Function 번호 (SxFy의 F)

    uint8_t  p_type;        // 프로토콜 타입 (0 = SECS-II)
    uint8_t  s_type;        // 메시지 타입 (data / control 구분)

    uint32_t system_bytes;  // 요청-응답 매칭용 ID
} HSMSHeader;
#pragma pack(pop)


#endif