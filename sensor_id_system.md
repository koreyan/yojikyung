| Module ID | Sensor ID | Full ID | 모듈 명칭        | 센서 타입             | 수량(Qty) | 하한값 (Min) | 상한값 (Max) | 단위    |
|-----------|-----------|---------|--------------|-------------------|---------|-----------|-----------|-------|
| 0x0       | 0x0       | 0x00    | Bond Head    | Temperature       | 6       | 60        | 150       | °C    |
| 0x0       | 0x2       | 0x02    | Bond Head    | Force             | 6       | 0.2       | 5         | N     |
| 0x0       | 0x3       | 0x03    | Bond Head    | Ultrasonic Power  | 4       | 0.5       | 5         | W     |
| 0x0       | 0x8       | 0x08    | Bond Head    | Vibration         | 4       | 0         | 0.3       | g     |
| 0x1       | 0x4       | 0x14    | Stage        | Position Encoder  | 14      | 0         | 300       | mm    |
| 0x1       | 0x5       | 0x15    | Stage        | Motor Current     | 6       | 0.2       | 2.5       | A     |
| 0x1       | 0x6       | 0x16    | Stage        | Motor Voltage     | 4       | 24        | 48        | V     |
| 0x1       | 0x7       | 0x17    | Stage        | Motor Speed       | 4       | 0         | 500       | mm/s  |
| 0x1       | 0x8       | 0x18    | Stage        | Vibration         | 4       | 0         | 0.2       | g     |
| 0x2       | 0x0       | 0x20    | Heater       | Temperature       | 8       | 100       | 250       | °C    |
| 0x2       | 0xF       | 0x2F    | Heater       | Power Consumption | 4       | 200       | 1000      | W     |
| 0x3       | 0x9       | 0x39    | Vacuum       | Vacuum Pressure   | 8       | -80       | -30       | kPa   |
| 0x3       | 0xA       | 0x3A    | Vacuum       | Flow              | 4       | 5         | 50        | L/min |
| 0x4       | 0x5       | 0x45    | Motor/Drive  | Motor Current     | 10      | 0.5       | 5         | A     |
| 0x4       | 0x6       | 0x46    | Motor/Drive  | Motor Voltage     | 6       | 24        | 48        | V     |
| 0x4       | 0x7       | 0x47    | Motor/Drive  | Motor Speed       | 6       | 0         | 3000      | rpm   |
| 0x4       | 0x8       | 0x48    | Motor/Drive  | Vibration         | 4       | 0         | 0.4       | g     |
| 0x5       | 0xB       | 0x5B    | Vision       | Vision Alignment  | 5       | -5        | 5         | µm    |
| 0x5       | 0xC       | 0x5C    | Vision       | Defect Detection  | 5       | 0         | 1         | -     |
| 0x6       | 0x0       | 0x60    | Environment  | Temperature       | 5       | 20        | 25        | °C    |
| 0x6       | 0xD       | 0x6D    | Environment  | Humidity          | 2       | 30        | 50        | %RH   |
| 0x6       | 0xE       | 0x6E    | Environment  | Airflow           | 3       | 0.3       | 1         | m/s   |
| 0x7       | 0xF       | 0x7F    | Power System | Power Consumption | 6       | 2         | 10        | kW    |
