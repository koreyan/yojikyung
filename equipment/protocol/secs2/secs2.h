#ifndef SECS2_H
#define SECS2_H

#include "common.h"

//------------------------------------------------------------
// SECS-II Format Code (Length Byte = 1 기준)
//------------------------------------------------------------

// List
#define SECS_L      0x01

// Binary
#define SECS_B      0x21

// Boolean
#define SECS_BOOL   0x25

// ASCII
#define SECS_A      0x41

// Signed Integer
#define SECS_I1     0x65
#define SECS_I2     0x69
#define SECS_I4     0x71
#define SECS_I8     0x61

// Floating Point
#define SECS_F4     0x91
#define SECS_F8     0x81

// Unsigned Integer
#define SECS_U1     0xA5
#define SECS_U2     0xA9
#define SECS_U4     0xB1
#define SECS_U8     0xA1


//------------------------------------------------------------
// Format Code + Length Byte 조합
//------------------------------------------------------------
//  SECS-II Item의 "타입 + Length Byte 정보"를 만드는 1바이트
#define SECS_ITEM(type, lb)   ((type) + ((lb) - 1))



// SECS-II의 [Format+LB][Length] 영역을 생성하는 함수
uint8_t* write_item_header(uint8_t* p, uint8_t type, uint32_t len);
//  SECS-II Length 필드를 big-endian으로 기록
uint8_t* write_length(uint8_t* p, uint32_t len, uint8_t lb);
// LIST 시작, 하위 요소 개수 지정
uint8_t* write_list(uint8_t* p, uint32_t count);




//------------------------------------------------------------
//  SECS-II로 인코딩 
//------------------------------------------------------------

// 모든 숫자 타입을 big-endian으로 변환하여 SECS-II로 인코딩
uint8_t* write_primitive_be(uint8_t* p, uint8_t type, uint64_t value, uint32_t size);
// uint16 값을 U2 타입으로 인코딩
uint8_t* write_u2(uint8_t* p, uint16_t v);
// uint64 값을 U8 타입으로 인코딩
uint8_t* write_u8(uint8_t* p, uint64_t v);
// float 값을 F4 타입으로 인코딩
uint8_t* write_f4(uint8_t* p, float v);
#endif