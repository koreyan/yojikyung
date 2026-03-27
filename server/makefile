# =========================
# Compiler
# =========================
CC = gcc
CFLAGS = -Wall -O2

# =========================
# Target
# =========================
TARGET = server

# =========================
# Source Files
# =========================

SRCS = \
main.c \
controller/controller.c \
decoder/decoder.c \
decoder/hsms_header_parser/hsms_header_parser.c \
decoder/secs2_parser/secs2_parser.c \
network/network.c \
network/hsms_receiver/hsms_receiver.c \
run_server/run_server.c

# =========================
# Object Files
# =========================

OBJS = $(SRCS:.c=.o)

# =========================
# Include Path
# =========================

INCLUDES = -Icommon \
           -Icontroller \
           -Idecoder \
           -Idecoder/hsms_header_parser \
           -Idecoder/secs2_parser \
           -Inetwork \
           -Inetwork/hsms_receiver \
           -Irun_server\
           -Itype\

# =========================
# Build
# =========================

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# =========================
# Compile Rule
# =========================

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# =========================
# Clean
# =========================

clean:
	rm -f $(OBJS) $(TARGET)

# =========================
# Rebuild
# =========================

re: clean all