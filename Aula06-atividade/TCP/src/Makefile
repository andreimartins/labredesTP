# Makefile for TCP socket project

CC = gcc
CFLAGS = -Wall -Wextra -g -Itools
SRC_DIR = .
OBJ_DIR = obj
BIN_DIR = bin

CLIENT_SRC = tcp_client.c
SERVER_SRC = tcp_server.c
LOG_SRC = tools/log.c
UTILS_SRC = tools/Utils.c

CLIENT_OBJ = $(OBJ_DIR)/tcp_client.o $(OBJ_DIR)/log.o $(OBJ_DIR)/Utils.o
SERVER_OBJ = $(OBJ_DIR)/tcp_server.o $(OBJ_DIR)/log.o $(OBJ_DIR)/Utils.o

CLIENT_BIN = tcp_client
SERVER_BIN = tcp_server


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




clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR) $(CLIENT_BIN) $(SERVER_BIN)

.PHONY: all clean