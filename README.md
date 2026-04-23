실시간 반도체 센서 데이터 처리 시스템 '요지경(Yojikyung)' 프로그램 설계도

1. 시스템 아키텍처 개요 및 설계 비전

본 시스템은 반도체 제조 공정의 핵심인 수율 관리 시스템(YMS)의 요구사항을 충족하기 위해 설계되었습니다. 
초미세 공정에서는 밀리초(ms) 단위의 데이터 지연이 수억 원 규모의 웨이퍼 손실로 이어질 수 있습니다. 따라서 본 아키텍처는 결정론적 지연 시간(Deterministic Latency) 확보와 초당 데이터 처리량(Throughput) 극대화를 최우선 과제로 삼습니다.

시스템은 임베디드 제어와 고속 데이터 분석에 최적화된 C 언어를 기반으로 하며, 네트워크 지터(Jitter) 및 메모리 단편화(Fragmentation) 리스크를 최소화하기 위해 **3계층 구조(3-Tier Architecture)**를 채택합니다.

* 장비 시뮬레이터 계층 (Equipment Simulator): 128개 센서의 독립적 주기성을 모델링하며, 데이터 정합성을 보장하는 HSMS/SECS-II 표준 프로토콜 패킷을 생성합니다.
* 데이터 처리 서버 계층 (C Server): 서브 밀리초 단위의 연산을 수행하는 시스템의 중추입니다. 패킷 재조립, 디코딩, 실시간 분석(이동 평균 및 이상 탐지), 환형 버퍼(Circular Buffer) 기반의 10분 데이터 관리를 수행합니다.
* 모니터링 계층 (Monitoring Program): 분석된 결과를 JSON 스트림으로 수신하여 가시화합니다. 엔지니어의 인지 부하를 줄이기 위해 Industrial Dark Mode 기반의 대시보드를 제공합니다.

시스템 계층 구조 및 데이터 흐름 (Project Structure)

real_time_data_processing_system
├── equipment (Simulator Tier)
│   ├── data_generator/ (Logic: Delta Ratio 1%, Anomaly 0.01%)
│   ├── protocol/ (SECS-II S6F1, HSMS Standard)
│   └── run/ (8-Module Multi-threading, Socket Mutex)
├── server (Processing Tier)
│   ├── network/hsms_receiver/ (4096-byte Fixed Frame Reassembly)
│   ├── decoder/ (HSMS Header & SECS-II Body Parsing)
│   ├── sensor_dispatcher/ (Moving Average Window 5, Anomaly Detection)
│   ├── storage/ (RAM Circular Buffer: 650,000 Records)
│   └── monitor_server/ (Non-blocking JSON Broadcast)
└── monitoring (Visualization Tier)
    └── UI/UX (Industrial Dark Mode, Chart.js, Status Tiles)  (아직 구현 안 됨)



--------------------------------------------------------------------------------


2. 엔드-투-엔드(End-to-End) 데이터 흐름 설계

데이터는 생성 시점부터 시각화까지 엄격한 직렬화 및 변환 과정을 거치며, 각 단계는 데이터 무결성과 전송 효율을 극대화하도록 설계되었습니다.

데이터 생명주기 및 전략적 변환 단계

1. Data Generation: data_generator.c에서 DELTA_RATIO(0.01f)를 적용하여 이전 값 대비 1% 내외의 자연스러운 변화를 생성합니다. ABNORMAL_PROB(0.0001f)에 따라 0.01% 확률로 임계치를 벗어나는 이상치를 발생시켜 실하중 환경을 모사합니다.
2. Packetization: packet.c가 생성된 데이터를 S6F1(Data Trace Report) 메시지로 구성합니다. 타임스탬프, 센서 카운트, 데이터 리스트를 SECS-II 포맷으로 구조화합니다.
3. Transmission: TCP 소켓을 통해 4096바이트 고정 크기 패킷을 전송합니다. 이는 네트워크 파편화를 방지하고 서버 측 수신 로직의 예측 가능성을 높이기 위한 전략적 결정입니다.
4. Reception & Reassembly: hsms_receiver.c의 RecvBuffer는 TCP 파편화(Fragmentation)로 인해 쪼개져 들어오는 데이터를 4096바이트가 충족될 때까지 적치하여 완전한 프레임을 재조립합니다.
5. Decoding & Processing: decoder.c가 Big-endian 데이터를 Host-order로 변환합니다. moving_average.c는 WINDOW_SIZE 5를 사용하여 노이즈를 평활화하며, anomaly_detection.c는 임계값을 대조합니다.
6. Dispatch & Storage: 분석 결과는 history_storage.c의 환형 버퍼에 저장됨과 동시에 json_builder.c를 통해 직렬화되어 모니터링 클라이언트로 브로드캐스트됩니다.

데이터 상태 및 엔디안 변환 요약

단계	데이터 형태	주요 특징	엔디안 (Endian)
생성	SensorData Struct	Raw Value, 1% 변화율 적용	Host (Little-endian)
패킹	S6F1 SECS-II	표준 메시지 구조화	Big-endian (Network)
캡슐화	HSMS Header	14바이트 제어 정보 포함	Big-endian (Network)
분석	PacketData Struct	이동 평균(Window: 5) 산출	Host (Little-endian)
시각화	JSON String	웹 기반 호환성 및 직렬화	N/A (Text-based)


--------------------------------------------------------------------------------


3. 통신 프로토콜 및 인터페이스 명세

산업 표준인 HSMS/SECS-II를 채택하여 실제 반도체 장비와의 호환성을 확보하고 통신 신뢰성을 보장합니다.

장비(Equipment) ↔ 서버(Server) 인터페이스

* HSMS Header (14 Bytes): 모든 메시지의 접두어로 사용되며 hsms.h에 엄격히 정의됩니다.
  * Length (4B): 헤더를 제외한 메시지 바디의 총 길이.
  * Session ID (2B): 장비와 서버 간 논리적 연결 식별자.
  * Stream(1B) / Function(1B): 메시지 유형 정의 (본 시스템은 S6F1 주력).
  * P-Type (1B) / S-Type (1B): 프레젠테이션 및 세션 유형 정의.
  * System Bytes (4B): 메시지의 유일성 보장 및 트랜잭션 추적.
* SECS-II Payload 인코딩: secs2.c 모듈을 통해 정밀 데이터 타입을 인코딩합니다.
  * U2(2B): 센서 카운트 등 정수형 데이터.
  * U8(8B): ms 단위의 정밀 타임스탬프.
  * F4(4B): IEEE 754 표준 4바이트 부동 소수점 센서 값.

서버(Server) ↔ 모니터링(Monitoring) 인터페이스

* Non-blocking TCP Broadcast: monitor_server.c는 select()를 활용하여 다수의 모니터링 클라이언트에 데이터를 실시간 전송합니다. 지연 방지를 위해 비차단 소켓 모드로 운영됩니다.


--------------------------------------------------------------------------------


4. 구성 요소별 데이터 생성 및 처리 로직 상세

4.1 반도체 장비 시뮬레이터 (Equipment Simulator)

* Concurrency 전략: 128개 센서를 8개의 주요 공정 모듈(Bond Head, Stage, Heater 등)로 그룹화하여 독립적인 pthread에서 실행합니다.
* Socket Atomicity 보장: 여러 스레드가 동시에 소켓에 접근할 때 패킷 바이트가 뒤섞이는 것을 방지하기 위해 pthread_mutex를 사용하여 4096바이트 프레임 단위의 원자적(Atomic) 전송을 강제합니다.
* 데이터 생성 순서: 모듈 스레드 기동 → 담당 센서 리스트 순회 → 각 센서별 개별 주기(Sampling Rate) 확인 → 주기가 도래한 센서만 선택적으로 데이터 생성(update_sensor_value) → 뮤텍스 점유 후 패킷 송신.

4.2 데이터 처리 서버 (C Server)

* Fixed-Frame Receiver: hsms_receiver.c는 고정 크기 패킷 방식을 사용하여 동적 메모리 할당을 배제합니다. RecvBuffer의 write_pos를 추적하여 4096바이트 미만의 파편화된 데이터는 누적하고, 패킷이 완성되는 즉시 디코딩 파이프라인으로 푸시합니다.
* 데이터 분석 알고리즘:
  * Moving Average: SensorState 구조체 내의 window[5] 배열과 sum 변수를 활용하여 O(1) 복잡도로 이동 평균을 업데이트합니다.
  * Anomaly Detection: sensor_class.h에 정의된 각 센서별 고유 Min/Max 임계값과 비교하여 즉각적인 Flag를 생성합니다.
* Storage Management (Circular Buffer): history_storage.c는 MAX_HISTORY 650,000 레코드 규모의 환형 버퍼를 운용합니다.
  * Head/Tail Logic: 새로운 데이터는 head에 삽입되며, 10분(RETENTION_MS)이 경과한 데이터는 tail 이동을 통해 논리적으로 자동 삭제되어 메모리 효율을 극대화합니다.

4.3 모니터링 프로그램 (Visualization) (추가 필요)

* Layout 설계:
  * 128-Sensor Summary Tiles: 모든 센서의 현재 상태를 Green/Yellow/Red 색상 타일로 한눈에 모니터링.
  * Detailed Log Graphs: 특정 타일 클릭 시 해당 센서의 최근 10분간의 흐름을 Chart.js 동적 그래프로 렌더링.
* UI/UX: 현장 엔지니어의 피로도 감소를 위한 Industrial Dark Mode 및 상태 기반 즉각적 알림 시스템.


--------------------------------------------------------------------------------


5. 시스템 성능 지표 및 구현 제약 사항

본 시스템은 고부하 환경에서도 안정적인 운용이 가능하도록 다음과 같은 기술적 제약을 준수합니다.

* 처리 성능 목표: 128개 센서가 1ms 주기로 데이터를 생성하는 극한 상황에서도 데이터 유실(Drop) 없는 100% Throughput을 보장합니다.
* 메모리 최적화: 10분간의 히스토리를 보관하기 위해 약 480MB의 RAM을 선점(Pre-allocation) 방식으로 사용합니다. 이는 런타임 중의 Heap 할당 오버헤드를 제거하여 시스템의 실시간성을 확보하기 위함입니다.
* 자원 관리: HistoryStorage의 환형 버퍼 구조를 통해 지속적인 데이터 유입 환경에서도 추가적인 메모리 복사 없이 일정한 성능을 유지합니다.

본 설계도는 '요지경' 시스템의 개발 지침서로서, 모든 모듈은 본 문서에 명시된 규격과 임계치 상수를 엄격히 준수하여 일관된 성능을 발현해야 합니다.
