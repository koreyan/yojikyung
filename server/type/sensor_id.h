#ifndef SENSOR_ID_H
#define SENSOR_ID_H

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


#endif