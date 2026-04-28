#include "common.h"

// 시스템 시간을 millisecond 단위로 반환
uint64_t get_time_ms();

// min ~ max 사이의 랜덤 float 생성
// → 초기값, 변화량 계산에 사용
float rand_float(float min, float max);

// 값이 범위를 벗어나지 않도록 제한
// → 센서 값이 정상 범위 유지하도록 보정
float clamp(float v, float min, float max);