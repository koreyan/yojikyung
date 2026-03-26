#include "secs2.h"

// SECS-II의 [Format+LB][Length] 영역을 생성하는 함수
uint8_t* write_item_header(uint8_t* p, uint8_t type, uint32_t len)
{
    uint8_t lb;

    if (len <= 0xFF)        lb = 1;
    else if (len <= 0xFFFF) lb = 2;
    else                    lb = 3;

    *p++ = SECS_ITEM(type, lb);  // 타입 + Length Byte 개수 인코딩
    p = write_length(p, len, lb); // 실제 데이터 길이를 big-endian으로 기록

    return p;
}

//  SECS-II Length 필드를 big-endian으로 기록
uint8_t* write_length(uint8_t* p, uint32_t len, uint8_t lb)
{
    for (int i = lb - 1; i >= 0; i--) {
        p[i] = len & 0xFF;
        len >>= 8;
    }
    return p + lb;
}


// 모든 숫자 타입을 big-endian으로 변환하여 SECS-II로 인코딩
uint8_t* write_primitive_be(uint8_t* p, uint8_t type, uint64_t value, uint32_t size)
{
    uint8_t buf[8];

    for (int i = 0; i < size; i++) {
        buf[size - 1 - i] = (value >> (8 * i)) & 0xFF;
    }

    p = write_item_header(p, type, size); // 타입 + 길이
    memcpy(p, buf, size);                 // 데이터 복사

    return p + size;
}

// LIST 시작, 하위 요소 개수 지정
uint8_t* write_list(uint8_t* p, uint32_t count)
{
    return write_item_header(p, SECS_L, count);
}

// uint16 값을 U2 타입으로 인코딩
uint8_t* write_u2(uint8_t* p, uint16_t v)
{
    return write_primitive_be(p, SECS_U2, v, 2);
}

// uint64 값을 U8 타입으로 인코딩
uint8_t* write_u8(uint8_t* p, uint64_t v)
{
    return write_primitive_be(p, SECS_U8, v, 8);
}

// float 값을 F4 타입으로 인코딩
uint8_t* write_f4(uint8_t* p, float v)
{
    uint32_t tmp;
    memcpy(&tmp, &v, 4);
    return write_primitive_be(p, SECS_F4, tmp, 4);
}