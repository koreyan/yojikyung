CC = gcc

CFLAGS = -Wall -Wextra -O2 -pthread \
         -Icommon -Inetwork -Ipacket -Isensor -Iutil

LDFLAGS = -pthread

TARGET = equipment

SRCS = main.c \
       $(wildcard common/*.c) \
       $(wildcard network/*.c) \
       $(wildcard packet/*.c) \
       $(wildcard sensor/*.c) \
       $(wildcard util/*.c) \
# 	   $(wildcard debug/*.c)

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all