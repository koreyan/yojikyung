#ifndef SENSORPROTOCOL_H
#define SENSORPROTOCOL_H
#include "common.h"

#define MODULE_SHIFT 12
#define TYPE_SHIFT 8

#define MAKE_CLASS_ID(module, type)  (((module) << 4) | (type))
#define MAKE_SENSOR_ID(m,t,i)(((m & 0x0F) << 12) | ((t & 0x0F) << 8) | (i & 0xFF))
#define GET_CLASS_ID(sensor_id) ((uint8_t)((sensor_id) >> 8))

// 장비 내부 모듈 종류
typedef enum {

    MODULE_BOND_HEAD = 0,   // Bond Head
    MODULE_STAGE,           // Stage
    MODULE_HEATER,          // Heater
    MODULE_VACUUM,          // Vacuum system
    MODULE_MOTOR_DRIVE,     // Motor / Drive
    MODULE_VISION,          // Vision system
    MODULE_ENVIRONMENT,     // Environment sensors
    MODULE_POWER            // Power system

} Module;


// 센서 측정 타입
typedef enum {

    SENSOR_TEMPERATURE = 0,      // temperature
    SENSOR_PRESSURE,             // pressure
    SENSOR_FORCE,                // force
    SENSOR_ULTRASONIC_POWER,     // ultrasonic power
    SENSOR_POSITION_ENCODER,     // position encoder
    SENSOR_MOTOR_CURRENT,        // motor current
    SENSOR_MOTOR_VOLTAGE,        // motor voltage
    SENSOR_MOTOR_SPEED,          // motor speed
    SENSOR_VIBRATION,            // vibration
    SENSOR_VACUUM_PRESSURE,      // vacuum pressure
    SENSOR_FLOW,                 // flow
    SENSOR_VISION_ALIGNMENT,     // vision alignment
    SENSOR_DEFECT_DETECTION,     // defect detection
    SENSOR_HUMIDITY,             // humidity
    SENSOR_AIRFLOW,              // airflow
    SENSOR_POWER_CONSUMPTION     // power consumption

} SensorType;


//------------------------------------------------------------
// 센서 데이터 구조
//------------------------------------------------------------
typedef struct {

    uint16_t sensor_id;     // 센서 ID  [Module|SensorType|Index]
    float value;            // 센서 측정값

} SensorData;



//------------------------------------------------------------
// 센서 클래스 (정상 범위 정의)
//------------------------------------------------------------
typedef struct {

    uint8_t class_id;       // [Module(4bit)][SensorType(4bit)]
    float normal_min;       // 정상 최소값
    float normal_max;       // 정상 최대값

} SensorClass;


//------------------------------------------------------------
// 센서 인스턴스 관리 구조
//------------------------------------------------------------
typedef struct {

    uint16_t sensor_id;     // [Module|SensorType|Index]

    uint32_t period_ms;     // 데이터 생성 주기
    uint64_t next_time_ms;  // 다음 데이터 생성 시각

} Sensor;

// Sensor 객체 데이터 생성 주기를 보고 주기가 되면 SensorData를 만들면 됨. 굳이 내포하고 있을 필요없음.


#endif