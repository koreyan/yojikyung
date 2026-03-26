#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

/* ============================= */
/* 🔹 HSMS Header 구조 (14 bytes) */
/* ============================= */
#pragma pack(push, 1)
typedef struct {
    uint32_t length;        // Body 길이
    uint16_t session_id;
    uint8_t  stream;
    uint8_t  function;
    uint8_t  p_type;
    uint8_t  s_type;
    uint32_t system_bytes;  // 메시지 ID
} HSMSHeader;
#pragma pack(pop)

/* ============================= */
/* 🔹 센서 데이터 구조 */
/* ============================= */
typedef struct {
    uint16_t sensor_id;     // [Module|SensorType|Index]
    float value;
} SensorData;

/* ============================= */
/* 🔹 Sensor 정의 테이블 */
/* ============================= */
typedef struct {
    uint8_t module;
    uint8_t type;
    const char *module_name;
    const char *sensor_name;
    const char *unit;
} SensorInfo;

SensorInfo sensor_table[] = {
    {0x0,0x0,"Bond Head","Temperature","°C"},
    {0x0,0x2,"Bond Head","Force","N"},
    {0x0,0x3,"Bond Head","Ultrasonic Power","W"},
    {0x0,0x8,"Bond Head","Vibration","g"},
    {0x1,0x4,"Stage","Position Encoder","mm"},
    {0x1,0x5,"Stage","Motor Current","A"},
    {0x1,0x6,"Stage","Motor Voltage","V"},
    {0x1,0x7,"Stage","Motor Speed","mm/s"},
    {0x1,0x8,"Stage","Vibration","g"},
    {0x2,0x0,"Heater","Temperature","°C"},
    {0x2,0xF,"Heater","Power Consump","W"},
    {0x3,0x9,"Vacuum","Vacuum Pressure","kPa"},
    {0x3,0xA,"Vacuum","Flow","L/min"},
    {0x4,0x5,"Motor/Drive","Motor Current","A"},
    {0x4,0x6,"Motor/Drive","Motor Voltage","V"},
    {0x4,0x7,"Motor/Drive","Motor Speed","rpm"},
    {0x4,0x8,"Motor/Drive","Vibration","g"},
    {0x5,0xB,"Vision","Vision Alignment","µm"},
    {0x5,0xC,"Vision","Defect Detection","-"},
    {0x6,0x0,"Environment","Temperature","°C"},
    {0x6,0xD,"Environment","Humidity","%RH"},
    {0x6,0xE,"Environment","Airflow","m/s"},
    {0x7,0xF,"Power System","Power Consump","kW"}
};

#define SENSOR_TABLE_SIZE (sizeof(sensor_table)/sizeof(sensor_table[0]))

/* ============================= */
/* 🔹 sensor_id 분해 함수 */
/* ============================= */
void parse_sensor_id(uint16_t id, int *module, int *type, int *index) {
    *module = (id >> 12) & 0x0F;
    *type   = (id >> 8) & 0x0F;
    *index  = id & 0xFF;
}

/* ============================= */
/* 🔹 sensor_id → 이름/단위 매핑 */
/* ============================= */
const SensorInfo* lookup_sensor(uint16_t sensor_id) {
    int m,t,i;
    parse_sensor_id(sensor_id,&m,&t,&i);

    for(int k=0;k<SENSOR_TABLE_SIZE;k++) {
        if(sensor_table[k].module == m && sensor_table[k].type == t)
            return &sensor_table[k];
    }
    return NULL; // 테이블에 없는 경우
}

/* ============================= */
/* 🔹 endian-safe 읽기 유틸 */
/* ============================= */
uint16_t read_u2(uint8_t *buf) { return (buf[0]<<8)|buf[1]; }
uint64_t read_u8(uint8_t *buf) {
    uint64_t val=0; for(int i=0;i<8;i++) val=(val<<8)|buf[i]; return val;
}
float read_f4(uint8_t *buf) { uint32_t tmp=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3]; float f; memcpy(&f,&tmp,sizeof(float)); return f; }

/* ============================= */
/* 🔹 SECS-II Body 파싱 */
/* ============================= */
int parse_secs_body(uint8_t *buf, int len) {
    int offset=0;

    offset++; offset++; // root list type/count

    offset+=2; // timestamp type/length
    uint64_t timestamp = read_u8(buf + offset); offset+=8;
    printf("Timestamp: %llu ms\n", (unsigned long long)timestamp);

    offset+=2; // sensor_count type/length
    uint16_t sensor_count = read_u2(buf + offset); offset+=2;
    printf("Sensor Count: %d\n", sensor_count);

    offset+=2; // sensor list type/count
    uint8_t list_count = buf[1]; // root list count ?? 간단화

    for(int i=0;i<sensor_count;i++) {
        offset+=2; // L[2] type/count

        offset+=2; // sensor_id type/length
        uint16_t sensor_id = read_u2(buf + offset); offset+=2;

        offset+=2; // value type/length
        float value = read_f4(buf + offset); offset+=4;

        const SensorInfo *info = lookup_sensor(sensor_id);
        if(info)
            printf("[%02d] %s/%s(ID=0x%04X) = %.3f %s\n",
                i, info->module_name, info->sensor_name, sensor_id, value, info->unit);
        else
            printf("[%02d] Unknown Sensor ID=0x%04X, Value=%.3f\n", i, sensor_id, value);
    }
    return 0;
}

/* ============================= */
/* 🔹 전체 패킷 파싱 */
/* ============================= */
void parse_packet(uint8_t *packet) {
    HSMSHeader *hdr = (HSMSHeader *)packet;
    uint32_t length = ntohl(hdr->length);

    printf("\n===== HSMS HEADER =====\n");
    printf("Length: %u, Stream: %d, Function: %d, System Bytes: %u\n",
        length,hdr->stream,hdr->function,ntohl(hdr->system_bytes));

    parse_secs_body(packet + sizeof(HSMSHeader), length);
}

/* ============================= */
/* 🔹 TCP 서버 열기 */
/* ============================= */
#define PORT 12345
void run_server() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen=sizeof(addr);
    uint8_t buffer[4096];

    server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0){perror("socket"); exit(1);}

    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(PORT);

    if(bind(server_fd,(struct sockaddr*)&addr,sizeof(addr))<0){perror("bind"); exit(1);}
    if(listen(server_fd,5)<0){perror("listen"); exit(1);}
    printf("Server listening on port %d...\n",PORT);

    client_fd=accept(server_fd,(struct sockaddr*)&addr,&addrlen);
    if(client_fd<0){perror("accept"); exit(1);}
    printf("Client connected\n");

    while(1){
        int n=recv(client_fd,buffer,sizeof(HSMSHeader),MSG_WAITALL);
        if(n<=0) break;
        HSMSHeader *hdr=(HSMSHeader*)buffer;
        uint32_t body_len=ntohl(hdr->length);

        n=recv(client_fd,buffer+sizeof(HSMSHeader),body_len,MSG_WAITALL);
        if(n<=0) break;

        parse_packet(buffer);
    }
    close(client_fd);
    close(server_fd);
}

/* ============================= */
/* 🔹 main */
/* ============================= */
int main() {
    run_server();
    return 0;
}