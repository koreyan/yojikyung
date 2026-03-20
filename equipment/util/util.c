#include "util.h"

// 현재 시간을 밀리초 단위로 반환하는 함수
uint64_t now_ms(){
    // timeval 구조체는 시간을 두 부분으로 나눠 저장
    // tv_sec : 초(second)
    // tv_usec: 마이크로초(microsecond, 백만분의 1초)
    struct timeval tv; 
    
    // 현재 시간을 tv 구조체에 저장
    // 첫 번째 인자: 시간을 저장할 구조체
    // 두 번째 인자: timezone 정보 (요즘은 사용하지 않으니까 NULL)
    gettimeofday(&tv, NULL);


    // 초 단위 시간을 밀리초로 변환 
    // tv_sec * 1000 -> 초 -> 밀리초
    // 
    // tv_usec은 마이크로초 -> 1000으로 나누면 밀리초
    // 예시 
    // tv_sec = 10
    // tv_usec = 500000
    //
    // 10 * 1000 = 10000 ms
    // 500000 / 1000 = 500ms
    //
    // 총 10500 ms
    // ULL = unsigned Long Long  => 큰 값 계산할 때 타입을 이렇게 강제. 
    return tv.tv_sec * 1000ULL + tv.tv_usec / 1000;
}

