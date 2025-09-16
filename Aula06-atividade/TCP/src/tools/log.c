#include "log.h"

const char* log_tag_str[] = {
    "INFO",
    "WARN",
    "ERROR"
};

void write_log(char* log_line, enum log_tag_t log_tag, char* log_name)
{
    char log_file[150] = {0};
    FILE *file;
    time_t now;
    struct tm *tm_info;
    char date_buffer[16] = {0};
    char time_buffer[16] = {0};

    // Get the current time
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(date_buffer, "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    sprintf(time_buffer, "%02d-%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(log_file, "%s%s-%s", LOG_DIR, date_buffer,log_name);

    
    verify_dir(LOG_DIR);

    if (access(log_file, F_OK) == 0) {
        file = fopen(log_file, "a");
     
    } else {
        file = fopen(log_file, "w");
        printf("Log file created: %s\n", log_file);
       
    }

    // Write the log entry
    char line[LOG_BUFFER_SIZE+32] = {0};
    snprintf(line, sizeof(line), "[%s]\t[%s]\t %s\n", time_buffer, log_tag_str[log_tag], log_line);

    //print on terminal
    printf("%s", line);

    //write on file
    fprintf(file, line);
    fflush(file); // Ensure the log entry is written immediately


    fclose(file);
    
}