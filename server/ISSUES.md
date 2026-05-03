# C Server 현재 문제점 분석서

> 분석 일시: 2026-04-29  
> 대상: `server/` 디렉터리 — 데이터 처리 서버 계층 (C Server)

---

## 문제 1: 장비 데이터 단절 시 서버 동반 종료

### 현상
장비(Equipment Simulator)의 TCP 연결이 끊어지면 서버 프로세스 전체가 종료됩니다.

### 현재 흐름
```
run() → accept_client() (1회) → run_client_loop() → 연결 끊김 → close → return 0 → 프로세스 종료
```

### 근본 원인

| 위치 | 코드 | 문제점 |
|------|------|--------|
| `run_server.c:33` | `int client_fd = accept_client(server_fd);` | 단 1회만 accept → 재연결 불가 |
| `run_server.c:15-17` | `if (ret <= 0) break;` | 연결 끊김 = 루프 즉시 종료 |
| `run_server.c:37-41` | `close()` → `return 0` | 루프 탈출 후 복구 없이 프로세스 종료 |

### 영향도
- **심각도: Critical**
- 장비 1대가 일시적으로 끊기면 서버 전체가 내려감
- 모니터링 클라이언트도 함께 연결 해제
- 환형 버퍼(히스토리 스토리지)의 데이터도 유실 (cleanup_storage 호출됨)
- 수동으로 서버를 재시작해야만 복구 가능

---

## 문제 2: 리눅스(GCP Ubuntu) 환경 호환성

### 현상
현재 macOS에서 개발/테스트되어 있으며, GCP Ubuntu에서 빌드 시 컴파일 오류 또는 런타임 크래시가 발생할 수 있습니다.

### 세부 항목

#### 2-1. `<stdint.h>` 누락
- **파일**: `common/common.h`
- **문제**: `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` 등이 프로젝트 전역에서 사용되지만 `<stdint.h>`를 include하지 않음
- **macOS**: 다른 헤더가 간접 포함시켜 줌
- **Ubuntu GCC**: strict 모드에서 컴파일 오류 발생

#### 2-2. `-lpthread` 링킹 플래그 누락
- **파일**: `makefile`
- **문제**: `file_logger.c`에서 `pthread_mutex_t`, `pthread_mutex_lock/unlock`을 사용하지만 링킹 시 `-lpthread` 플래그 없음
- **macOS**: libSystem에 기본 포함
- **Ubuntu**: 명시적 `-lpthread` 필수, 없으면 `undefined reference to pthread_mutex_lock` 링킹 오류

#### 2-3. `SIGPIPE` 시그널 미처리
- **파일**: 없음 (처리 코드 자체가 존재하지 않음)
- **문제**: 연결이 끊긴 소켓에 `send()` 호출 시 리눅스에서 `SIGPIPE` 시그널 발생 → 프로세스 즉사
- **영향 범위**: `monitor_server.c`의 `monitor_send_all()` 에서 모니터링 클라이언트가 비정상 종료된 상태에서 send 시 발생
- **macOS**: 발생 빈도 낮음
- **Ubuntu**: 매우 빈번하게 발생

#### 2-4. 이미 해결된 항목 (수정 불필요)
- ✅ `file_logger.c`: `_NSGetExecutablePath` / `readlink("/proc/self/exe")` 분기 → `#ifdef __APPLE__` 처리 완료
- ✅ `file_logger.c`: `F_FULLFSYNC` / `fsync` 분기 → `#ifdef __APPLE__` 처리 완료
- ✅ 소켓 API (`socket`, `bind`, `listen`, `accept`, `recv`, `send`) → POSIX 표준으로 Ubuntu 호환
- ✅ `select()`, `fcntl()` → POSIX 표준으로 Ubuntu 호환

---

## 요약

| 문제 | 심각도 | 상태 |
|------|--------|------|
| 장비 단절 시 서버 종료 | 🔴 Critical | 미해결 |
| `<stdint.h>` 누락 | 🟡 Medium | 미해결 |
| `-lpthread` 링킹 누락 | 🔴 High | 미해결 |
| `SIGPIPE` 미처리 | 🔴 High | 미해결 |
| macOS 전용 API 분기 | 🟢 Resolved | ✅ 해결됨 |
