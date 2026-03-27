# 컴파일러
CC = gcc

# 옵션
CFLAGS = -Wall -Wextra -O2

# include 경로
INCLUDES = \
	-Icommon \
	-Idata_generator \
	-Inetwork \
	-Ipacket \
	-Iprotocol/hsms \
	-Iprotocol/secs2 \
	-Irun \
	-Itype \
	-Iutil

# 실행파일
TARGET = equipment

# 소스 파일 (존재하는 .c만 정확히 포함)
SRCS = \
	$(wildcard data_generator/*.c) \
	$(wildcard network/*.c) \
	$(wildcard packet/*.c) \
	$(wildcard protocol/secs2/*.c) \
	$(wildcard run/*.c) \
	$(wildcard type/*.c) \
	$(wildcard util/*.c) \
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

# 디버그용
print:
	@echo "SRCS = $(SRCS)"