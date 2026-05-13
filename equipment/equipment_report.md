# 실시간 데이터 처리 시스템 (Equipment) 분석 보고서

이 보고서는 `real_time_data_processing_system`의 장비 시뮬레이터(Equipment) 프로그램에 대한 아키텍처, 데이터 생성 체계, 통신 프로토콜 및 데이터 흐름을 상세히 분석하여 설명한다.

---

## 1. 전체적인 아키텍처

본 프로그램은 반도체/디스플레이 제조 공정 장비의 센서 데이터를 실시간으로 시뮬레이션하고, 표준 통신 프로토콜인 SECS-II/HSMS를 통해 서버로 전송하는 고성능 시뮬레이터다.

### **멀티스레드 기반 분산 처리**
- **모듈별 독립 스레드**: 시스템 내부의 8개 주요 모듈(Bond Head, Stage, Heater 등)을 각각 독립적인 스레드로 구동한다.
- **동기화 및 공유 자원**: 모든 모듈 스레드는 하나의 네트워크 소켓(`global_sock`)을 공유하며, 데이터 전송 시 `pthread_mutex`를 사용하여 데이터 패킷의 원자성(Atomicity)과 안정성을 보장한다.
- **고정 크기 패킷**: 네트워크 전송 효율과 서버 측의 수신 안정성을 위해 모든 패킷은 **4096바이트 고정 크기**로 패딩되어 전송된다.

---

## 2. Data Generator 상세 설명

데이터 생성기는 장비의 물리적 상태를 시뮬레이션하며, 각 센서의 특성에 맞는 주기와 데이터 범위를 가진다.

### **2.1 Sensor ID 체계 (16-bit)**
`sensor_id`는 하드웨어 구조를 반영하여 계층적으로 정의된다.

| 비트 범위 | 필드명 | 설명 |
| :--- | :--- | :--- |
| **Bit 12 ~ 15** | **Module** | 장비 내 물리적 모듈 (최대 16개) |
| **Bit 8 ~ 11** | **Sensor Type** | 측정 항목 (Temp, Force, Pos 등) |
| **Bit 0 ~ 7** | **Index** | 해당 모듈/타입 내 개별 식별 번호 (최대 256개) |

### **2.2 센서 구성 및 주기 (Configuration)**
총 128개의 센서가 다음과 같이 구성되어 독립적인 주기로 데이터를 생성한다.

| 모듈 (Module) | 센서 타입 (Type) | 센서 수 | 주기 (ms) | 하한값 | 상한값 | 단위 | 설명 |
| :--- | :--- | :---: | :---: | :---: | :---: | :---: | :--- |
| **Bond Head** | Temperature | 6 | 100 | 60.0 | 150.0 | °C | 헤드 온도 |
| | Force | 6 | 50 | 0.2 | 5.0 | kgf | 가압력 |
| | Ultrasonic Power | 4 | 50 | 0.5 | 5.0 | W | 초음파 출력 |
| | Vibration | 4 | 200 | 0.0 | 0.3 | g | 진동 |
| **Stage** | Position Encoder | 14 | 20 | 0.0 | 300.0 | mm | 스테이지 위치 |
| | Motor Current | 6 | 50 | 0.2 | 2.5 | A | 모터 전류 |
| | Motor Voltage | 4 | 50 | 24.0 | 48.0 | V | 모터 전압 |
| | Motor Speed | 4 | 20 | 0.0 | 500.0 | mm/s | 모터 속도 |
| | Vibration | 4 | 200 | 0.0 | 0.2 | g | 진동 |
| **Heater** | Temperature | 8 | 200 | 100.0 | 250.0 | °C | 히터 온도 |
| | Power Consumption| 4 | 50 | 200.0 | 1000.0 | W | 전력 소모량 |
| **Vacuum** | Vacuum Pressure | 8 | 100 | -80.0 | -30.0 | kPa | 진공 압력 |
| | Flow | 4 | 200 | 5.0 | 50.0 | L/min | 가스 유량 |
| **Motor/Drive** | Motor Current | 10 | 50 | 0.5 | 5.0 | A | 구동 모터 전류 |
| | Motor Voltage | 6 | 50 | 24.0 | 48.0 | V | 구동 모터 전압 |
| | Motor Speed | 6 | 20 | 0.0 | 3000.0 | rpm | 구동 모터 속도 |
| | Vibration | 4 | 100 | 0.0 | 0.4 | g | 구동부 진동 |
| **Vision** | Vision Alignment | 5 | 100 | -5.0 | 5.0 | μm | 정렬 오차 |
| | Defect Detection | 5 | 200 | 0.0 | 1.0 | - | 결함 검출 여부 |
| **Environment**| Temperature | 5 | 1000 | 20.0 | 25.0 | °C | 외기 온도 |
| | Humidity | 2 | 1000 | 30.0 | 50.0 | %RH | 습도 |
| | Airflow | 3 | 1000 | 0.3 | 1.0 | m/s | 기류 |
| **Power** | Power Consumption| 6 | 500 | 2000.0 | 10000.0 | W | 장비 전체 전력 |

---

## 3. 통신 프로토콜 (Protocol)

본 시스템은 반도체 장비 표준 통신 프로토콜인 **SECS/GEM**의 하위 계층을 구현한다.

### **3.1 HSMS (High-Speed SECS Message Services)**
HSMS는 TCP/IP 네트워크 환경에서 SECS-II 메시지를 교환하기 위한 전송 프로토콜이다. 모든 HSMS 메시지는 **4바이트의 Length 영역**과 **10바이트의 Header 영역**으로 시작된다.

#### **HSMS 메시지 구조**

| 필드명 | 크기 (Byte) | 설명 |
| :--- | :---: | :--- |
| **Message Length** | 4 | 뒤따르는 10바이트 Header와 SECS-II Body의 합계 길이를 나타낸다. (Big-endian) |
| **Session ID** | 2 | 장치 간의 논리적 통신 세션을 식별하기 위한 ID다. |
| **Header Byte 2** | 1 | **Stream 번호**: SECS-II 메시지의 카테고리를 지정한다. (SxFy의 S) |
| **Header Byte 3** | 1 | **Function 번호**: 각 Stream 내의 세부 기능을 지정한다. (SxFy의 F) |
| **P-Type** | 1 | **Protocol Type**: 프로토콜 타입을 정의하며, `0`은 SECS-II 전송을 의미한다. |
| **S-Type** | 1 | **Session Type**: 메시지의 성격(Data, Select, Deselect 등)을 구분한다. `0`은 데이터 메시지다. |
| **System Bytes** | 4 | 요청-응답 매칭을 위한 고유 ID다. |

#### **본 시스템의 HSMS 구현 특징**
- **Big-endian 준수**: `length`, `session_id`, `system_bytes` 등 1바이트를 초과하는 모든 필드는 네트워크 바이트 순서(Big-endian)를 준수하여 인코딩된다.
- **고정 크기 패킷 송신**: 실제 메시지 크기와 관계없이 **4096바이트 고정 크기** 패킷을 전송하여 서버 측에서의 데이터 읽기 경계 처리를 단순화하고 정렬 안정성을 확보했다.

---

### **3.2 SECS-II (SEMI Equipment Communications Standard Part 2)**
SECS-II는 장비와 호스트 간에 교환되는 메시지의 본문(Body) 형식을 정의하는 표준 프로토콜이다. 모든 데이터는 'Item'이라는 단위로 구성되며, 각 Item은 자체적인 타입 정보와 길이를 포함한다.

#### **SECS-II Item 구조**
각 Item은 **Item Header**와 **Data** 영역으로 나뉜다.
1. **Format Code & Length Byte (1 Byte)**: 데이터의 타입(6-bit)과 길이 필드의 크기(2-bit)를 나타낸다.
2. **Length Field (1~3 Bytes)**: 실제 데이터 영역의 크기를 바이트 단위로 나타낸다.
3. **Data Area**: 실제 데이터 값이 위치하며, 멀티바이트 숫자는 Big-endian 형식을 따른다.

#### **본 시스템에서 사용되는 주요 Format Code**

| 타입명 | 코드 (Hex) | 설명 |
| :--- | :---: | :--- |
| **List (L)** | `0x01` | 서로 다른 타입의 Item들을 그룹화하는 트리 구조의 컨테이너다. |
| **Unsigned Integer (U8)** | `0xA1` | 8바이트 부호 없는 정수다. 본 시스템에서는 `Timestamp` 전송에 사용한다. |
| **Unsigned Integer (U2)** | `0xA9` | 2바이트 부호 없는 정수다. `Sensor ID` 및 `Data Count` 전송에 사용한다. |
| **Floating Point (F4)** | `0x91` | 4바이트 단정밀도 부동 소수점이다. 실제 `Sensor Value` 전송에 사용한다. |

#### **S6F1 (Trace Data Send) 메시지 상세 구조**
본 시스템은 실시간 센서 데이터를 보고하기 위해 S6F1 메시지를 사용하며, 다음과 같은 중첩 List 구조를 가진다.

- **Level 1 (Top List)**: 3개의 항목을 포함 (Timestamp, Count, Data List)
- **Level 2 (Data List)**: 현재 주기가 도래한 센서 개수만큼의 List를 포함
- **Level 3 (Data Item)**: 개별 센서의 [ID, Value] 쌍을 구성 (U2, F4)


---

## 4. 데이터 흐름 (Data Flow)

장비 시뮬레이터가 구동되어 데이터를 생성하고 서버로 송신하기까지의 전 과정은 다음과 같은 순서로 진행된다.

### **4.1 초기화 단계 (Initialization)**
프로그램 시작 시, `run()` 함수에서 센서 시스템의 기초를 구성한다.
1. **센서 메타데이터 정의 (`init_sensors`)**: 각 센서의 `sensor_id`, `period_ms`(주기)를 설정한다. 이때 각 센서의 첫 실행 시간(`next_time_ms`)을 랜덤하게 분산시켜 트래픽이 한꺼번에 몰리는 것을 방지한다.
2. **상태 메모리 할당 및 초기값 설정 (`init_sensor_states`)**: 센서의 실시간 값을 저장할 메모리를 할당한다. 정상 범위(Min/Max) 내에서 랜덤한 초기값을 부여하여 자연스러운 시작 상태를 만든다.
3. **네트워크 연결**: 서버와 TCP/IP 연결을 수립하고 전역 소켓 핸들(`global_sock`)을 확보한다.

### **4.2 데이터 생성 및 수집 (Generation & Collection)**
8개의 모듈 스레드(`module_worker`)가 독립적으로 루프를 돌며 데이터를 생성한다.
1. **주기 판정**: 현재 시간(`get_time_ms`)과 센서별 `next_time_ms`를 비교하여 전송 주기가 도래한 센서만 선별한다.
2. **값 업데이트 (`update_sensor_value`)**: 
   - 이전 값을 기반으로 미세 변화량(`DELTA_RATIO`)을 적용해 새로운 값을 계산한다.
   - 일정 확률(`ABNORMAL_PROB`)로 정상 범위를 벗어나는 이상치를 삽입하여 현실성을 높인다.
   - 계산된 새로운 값을 `SensorState` 배열에 즉시 반영한다.

### **4.3 패킷 캡슐화 (Encapsulation)**
수집된 데이터를 표준 규격에 맞춰 바이너리로 변환한다.
1. **SECS-II Body 빌드 (`build_secs_body`)**:
   - `write_list`, `write_u8`, `write_f4` 등의 함수를 사용하여 데이터를 SECS-II S6F1 형식의 바이너리로 인코딩한다.
   - 모든 수치 데이터는 Big-endian으로 변환한다.
2. **HSMS Header 빌드 (`build_hsms_header`)**:
   - 생성된 SECS-II Body의 길이를 계산하여 10바이트 HSMS 헤더를 구성한다.
   - 요청-응답 매칭을 위한 `system_bytes`를 1씩 증가시키며 할당한다.

### **4.4 전송 및 업데이트 (Transmission & Update)**
최종 패킷을 네트워크로 송출하고 다음 주기를 준비한다.
1. **임계 구역(Critical Section) 진입**: 여러 스레드가 동시에 소켓을 사용하는 것을 방지하기 위해 `pthread_mutex_lock`을 수행한다.
2. **패딩 및 송신 (`send_sensor_packet`)**:
   - 4096바이트 크기의 버퍼를 `0`으로 초기화(Padding)한다.
   - 앞부분에 [HSMS Header] + [SECS-II Body]를 순서대로 복사한다.
   - `send()` 함수를 통해 정확히 4096바이트를 서버로 전송한다.
3. **다음 전송 시점 갱신**: 전송이 완료된 각 센서의 `next_time_ms`에 자신의 주기(`period_ms`)를 더해 다음 실행 시점을 예약한다.
4. **자원 해제**: `pthread_mutex_unlock`을 호출하여 소켓 자원을 다른 스레드가 사용할 수 있도록 해제한다.

