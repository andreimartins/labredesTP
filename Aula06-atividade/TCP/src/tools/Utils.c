#include "Utils.h"
#include <dirent.h>
#include <stdbool.h>

void verify_dir(const char *path)
{
    struct stat st = {0};
    
    if (stat(path, &st) == -1) {
        if (mkdir(path, 0764) == -1) {
            perror("Failed to create directory");
            exit(EXIT_FAILURE);
        }
    }
}

bool verify_file_exists(const char *file_path)
{
    return access(file_path, F_OK) == 0;
}

void collect_tcp_info(int sockfd, char *buffer, size_t buffer_size)
{
    struct tcp_info info;
    socklen_t info_len = sizeof(info);
    if (getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, &info_len) == -1) 
    {
        perror("getsockopt TCP_INFO failed");
        snprintf(buffer, buffer_size, "TCP_INFO error\n");
        return;
    }
    snprintf(buffer, buffer_size,
        "RTT: %u us, cwnd: %u, ssthresh: %u, retrans: %u\n",
        info.tcpi_rtt,
        info.tcpi_snd_cwnd,
        info.tcpi_snd_ssthresh,
        info.tcpi_total_retrans);
}


