# 1. 빌드 및 실행 환경으로 Ubuntu 사용
FROM ubuntu:22.04

# 2. 필수 빌드 도구 설치 (gcc, make 등)
RUN apt-get update && apt-get install -y \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# 3. 작업 디렉토리 설정
WORKDIR /app

# 4. 소스 코드 복사
COPY . .

# 5. 서버 컴파일
RUN make clean all

# 6. 서버가 사용할 포트 개방 (장비용 12345, 모니터링용 9001)
EXPOSE 12345
EXPOSE 9001

# 7. 서버 실행
CMD ["./server"]
