# Makefile for TCP socket project

CC = gcc
CFLAGS = -Wall -Wextra -g -Itools
SRC_DIR = .
OBJ_DIR = obj
BIN_DIR = bin

CLIENT_SRC = client.c
SERVER_SRC = server.c
LOG_SRC = tools/log.c
UTILS_SRC = tools/Utils.c

CLIENT_OBJ = $(OBJ_DIR)/client.o $(OBJ_DIR)/log.o $(OBJ_DIR)/Utils.o
SERVER_OBJ = $(OBJ_DIR)/server.o $(OBJ_DIR)/log.o $(OBJ_DIR)/Utils.o

CLIENT_BIN = client
SERVER_BIN = server


all: $(CLIENT_BIN) $(SERVER_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(SERVER_BIN): $(SERVER_OBJ)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/log.o: tools/log.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/Utils.o: tools/Utils.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

debug: CFLAGS += -DDEBUG
# Run 'make debug' to compile with debug flags
debug: all





clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: all clean