CC = gcc
CFLAGS = -Iinclude
LDFLAGS = -lsndfile -lsamplerate -lasound -lm
SRC_DIR = src
OBJ_DIR = output
TARGET = $(OBJ_DIR)/jojoplayer

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

CFLAGS += -Wall -Wextra -Werror \
		  -Wformat=2 -Wshadow -Wwrite-strings -Wredundant-decls \
		  -Wmissing-declarations -Wno-attributes -Wno-format-nonliteral \
		  -g3

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR) $(TARGET)
