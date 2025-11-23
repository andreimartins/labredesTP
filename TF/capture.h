// capture.h
#ifndef CAPTURE_H
#define CAPTURE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <asm-generic/socket.h>

#define BUFFER_SIZE 65536 // Tamanho máximo do pacote

int setup_raw_socket(const char* ifname);
int capture_packet(int sock_fd, unsigned char *buffer);

#endif // CAPTURE_H