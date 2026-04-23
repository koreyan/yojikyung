#include "../network/network.h"
#include "../packet/packet.h"
#include "../data_generator/data_generator.h"
#include "../util/util.h"
#include <pthread.h>

#define MAX_SENSORS 128        // 총 센서 수
#define MAX_PACKET_SENSORS 128  // 한 패킷에 담을 수 있는 최대 센서 수
#define MICRO_SEC 100000

void run();