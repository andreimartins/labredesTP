#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>


#include "Utils.h"


#define LOG_BUFFER_SIZE 500
#define LOG_DIR "logs/"



//log block definition
typedef enum log_tag_t{
    LOG_INFO = 0,
    LOG_WARN,
    LOG_ERROR
} log_tag_t;

extern const char* log_tag_str[];



void write_log(char* log_line, enum log_tag_t log_tag, char* log_file);