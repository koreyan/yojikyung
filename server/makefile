# 컴파일러
CC = gcc

# 옵션 (디버깅용)
CFLAGS = -Wall -Wextra -g -O0 -DDEBUG

# include 경로
INCLUDES = \
	-Icommon \
	-Icontroller \
	-Icontroller/debug \
	-Idecoder \
	-Idecoder/hsms_header_parser \
	-Idecoder/secs2_parser \
	-Inetwork \
	-Inetwork/hsms_receiver \
	-Irun_server \
	-Itype \
	-Isensor_dispatcher \
	-Isensor_dispatcher/anomaly_detection \
	-Isensor_dispatcher/moving_average \
	-Isensor_dispatcher/sensor_state \
	-Imonitor_server \
	-Imonitor_server/json_builder

# 실행파일
TARGET = server

# 소스 파일
SRCS = \
	$(wildcard controller/*.c) \
	$(wildcard controller/debug/*.c) \
	$(wildcard decoder/*.c) \
	$(wildcard decoder/hsms_header_parser/*.c) \
	$(wildcard decoder/secs2_parser/*.c) \
	$(wildcard network/*.c) \
	$(wildcard network/hsms_receiver/*.c) \
	$(wildcard run_server/*.c) \
	$(wildcard sensor_dispatcher/*.c) \
	$(wildcard sensor_dispatcher/anomaly_detection/*.c) \
	$(wildcard sensor_dispatcher/moving_average/*.c) \
	$(wildcard sensor_dispatcher/sensor_state/*.c) \
	$(wildcard type/*.c) \
	$(wildcard monitor_server/*.c) \
	$(wildcard monitor_server/json_builder/*.c) \
	$(wildcard monitor_server/json_builder/*.c) \
	$(wildcard storage/*.c) \
	main.c

# 오브젝트
OBJS = $(SRCS:.c=.o)

# 기본 타겟
all: $(TARGET)

# 링크
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 컴파일
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 정리
clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

# 디버깅용
print:
	@echo "SRCS = $(SRCS)"