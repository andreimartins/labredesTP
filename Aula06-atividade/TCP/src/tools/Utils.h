#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <stdbool.h>


void verify_dir(const char *path);
void collect_tcp_info(int sockfd, char *buffer, size_t buffer_size);