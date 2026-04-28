#include "debug.h"

//============================================ 디 버 깅 ==================================================//
void print_hex(uint8_t *data, int len)
{
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);

        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

/* Module 이름 */
const char* get_module_name(int m)
{
    switch(m)
    {
        case 0: return "BondHead";
        case 1: return "Stage";
        case 2: return "Heater";
        case 3: return "Vacuum";
        case 4: return "Motor";
        case 5: return "Vision";
        case 6: return "Env";
        case 7: return "Power";
        default: return "Unknown";
    }
}

/* Type 이름 */
const char* get_type_name(int t)
{
    static const char* names[] = {
        "Temperature",        // 0
        "Pressure",           // 1
        "Force",              // 2
        "UltrasonicPower",    // 3
        "PositionEncoder",    // 4
        "MotorCurrent",       // 5
        "MotorVoltage",       // 6
        "MotorSpeed",         // 7
        "Vibration",          // 8
        "VacuumPressure",     // 9
        "Flow",               // 10
        "VisionAlignment",    // 11
        "DefectDetection",    // 12
        "Humidity",           // 13
        "Airflow",            // 14
        "PowerConsumption"    // 15
    };

    if (t >= 0 && t < 16)
        return names[t];

    return "Unknown";
}

/* 단위 */
const char* get_unit(int m, int t)
{
    if (m == 0 && t == 0) return "°C";
    if (m == 0 && t == 2) return "N";
    if (m == 1 && t == 5) return "A";
    if (m == 3 && t == 9) return "kPa";

    return "";
}

/* 센서 하나 출력 */
void print_sensor(SensorData *s, int idx)
{
    int m, t, i;
    decode_sensor_id(s->sensor_id, &m, &t, &i);

    printf("[%02d] M=%d T=%d I=%d  →  %.3f\n",
           idx, m, t, i, s->value);

    printf("      %s / %s\n",
           get_module_name(m),
           get_type_name(t));
}
//==============================================================================================//
