# ==============================
# Compiler & Flags
# ==============================
CC = gcc
CFLAGS = -Wall -Wextra -O2

# ==============================
# Directories
# ==============================
COMMON_DIR = common
DATA_GEN_DIR = data_generator
NETWORK_DIR = network
PACKET_DIR = packet
PROTOCOL_DIR = protocol
RUN_DIR = run
UTIL_DIR = util

# ==============================
# Source Files
# ==============================
SRCS = \
	main.c \
	$(DATA_GEN_DIR)/data_generator.c \
	$(NETWORK_DIR)/network.c \
	$(PACKET_DIR)/packet.c \
	$(PROTOCOL_DIR)/secs2.c \
	$(PROTOCOL_DIR)/sensor.c \
	$(UTIL_DIR)/util.c \
	$(RUN_DIR)/run_equipment.c

# ==============================
# Object Files
# ==============================
OBJS = $(SRCS:.c=.o)

# ==============================
# Include Paths
# ==============================
INCLUDES = \
	-I$(COMMON_DIR) \
	-I$(DATA_GEN_DIR) \
	-I$(NETWORK_DIR) \
	-I$(PACKET_DIR) \
	-I$(PROTOCOL_DIR) \
	-I$(RUN_DIR) \
	-I$(UTIL_DIR)

# ==============================
# Target
# ==============================
TARGET = equipment_simulator

# ==============================
# Build Rules
# ==============================
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ==============================
# Clean
# ==============================
clean:
	rm -f $(OBJS) $(TARGET)

# ==============================
# Rebuild
# ==============================
re: clean all