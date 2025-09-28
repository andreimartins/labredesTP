
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "./tools/log.h"

// Protocol operation type definitions
#define OP_LIST 0   //cmd list
#define OP_PUT 1    //cmd put
#define OP_QUIT 2   //cmd quit
#define OP_DATA 3   //data block -- internal use
#define OP_ERROR 4  //error message -- internal use
#define OP_OK 5     //ok message -- internal use


#define LOG_FILE_NAME "client.log" //log file name

#define BUFFER_SIZE 1460 // TCP typical MTU size
#define FILE_NAME_BUFFER 1024 // Buffer for file names

// Header structure for our application protocol
#pragma pack(1)
typedef struct
{
    char type;
    uint32_t payload_size;
} Header;
#pragma pack()


char *LOG_FILE[100] = {0 };

// log line buffer
char log_line[LOG_BUFFER_SIZE] = {0};
void exit_error(char *buff);
bool cmd_list(int sock_fd);
bool cmd_put(int sock, const char *filename);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]); // Options to specify server IP and port
        write_log("Usage: <program> <host> <port>", LOG_ERROR, LOG_FILE);
        return 1;
    }

    //get pid for log file name
    pid_t pid = getpid();
    sprintf(LOG_FILE, "client-%d.log", pid);
    sprintf(log_line, "Client PID: %d", pid);
    write_log(log_line, LOG_INFO, LOG_FILE);

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock_fd;
    struct sockaddr_in server_addr;

    // Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("Could not create socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Connect to server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock_fd);
        return 1;
    }

    printf("Connected to server %s:%d\n", server_ip, server_port);

    // CLI
    char command[100], arg[512]; //512 for long paths
    while (1)
    {

        write_log("Available commands: list, put <filename>, quit\n", LOG_INFO, LOG_FILE);
        write_log("Enter a command: ", LOG_INFO, LOG_FILE);
        scanf("%s", command); // read command 

        Header header;
        if (strcmp(command, "list") == 0) // 'list' command
        { 

            cmd_list(sock_fd);
        }
        else if (strcmp(command, "put") == 0) // 'put' command
        { 
            scanf("%s", arg); // read argument (filename) from command put

            // Log start time for PUT command
            time_t start_time = time(NULL);
            write_log("Starting PUT command", LOG_INFO, LOG_FILE);
            cmd_put(sock_fd, arg);

            // Log end time for PUT command
            time_t end_time = time(NULL);
            double duration = difftime(end_time, start_time);
            sprintf(log_line, "Connection for PUT lasted approximately %.2f seconds.", duration);
            write_log(log_line, LOG_INFO, LOG_FILE);
        }
        else if (strcmp(command, "quit") == 0) // 'quit' command
        { 
            header.type = OP_QUIT;
            header.payload_size = 0;
            write(sock_fd, &header, sizeof(Header));
            break;
        }
        else
        {

            write_log("Unknown command", LOG_WARN, LOG_FILE);
        }
    }

    close(sock_fd);

    write_log("Connection closed", LOG_INFO, LOG_FILE);
    return 0;
}

void exit_error(char *buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}

bool cmd_list(int sock_fd)
{

    Header header;
    header.type = OP_LIST;
    header.payload_size = 0;
    write(sock_fd, &header, sizeof(Header));

    // Receive response
    if (read(sock_fd, &header, sizeof(Header)) > 0)
    {
        header.payload_size = ntohl(header.payload_size);
        char *file_list = malloc(header.payload_size + 1);
        read(sock_fd, file_list, header.payload_size);
        file_list[header.payload_size] = '\0';
        printf("--- Files on server ---\n%s----------------------------\n", file_list);
        free(file_list);
    }
    return true;
}

bool cmd_put(int sock, const char *filename_full)
{

    // Open file full path and check if exists

    FILE *fp = fopen(filename_full, "rb");
    if (fp == NULL)
    {
        perror("Error opening local file");
        return false;
    }

    // Split the filename from the path
    // example: /path/to/file.txt -> file.txt
    char *filename = strrchr(filename_full, '/');// Find last occurrence of '/'
    if (filename)
    {
        filename++; // Move past the '/'
    }
    else
    {
        filename = filename_full; // No path, use the whole filename
    }

    write_log("Opened file successfully", LOG_INFO, LOG_FILE);

    sprintf(log_line, "Sending file: %s", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);

    //send only the filename  without path
    Header put_header;
    put_header.type = OP_PUT;
    put_header.payload_size = htonl(strlen(filename));
    write(sock, &put_header, sizeof(Header));
    write(sock, filename, strlen(filename));

    // Check for server error (duplicate file || any error)
    Header response_header;
    if (read(sock, &response_header, sizeof(Header)) > 0) //read the header response
    {
        if (response_header.type == OP_ERROR)
        {
            response_header.payload_size = ntohl(response_header.payload_size);
            char *err_msg = malloc(response_header.payload_size + 1);
            read(sock, err_msg, response_header.payload_size); //read the message description
            err_msg[response_header.payload_size] = '\0';
            sprintf(log_line, "Server error: %s", err_msg);
            write_log(log_line, LOG_ERROR, LOG_FILE);
            free(err_msg);
            fclose(fp);
            return false;
        }
        else if (response_header.type == OP_OK)
        {
            write_log("OK to send", LOG_INFO, LOG_FILE);
        }
    }
    else
    {
        write_log("No response from server", LOG_ERROR, LOG_FILE);
        fclose(fp);
        return false;
    }

    // Send the file in blocks

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    Header data_header;
    data_header.type = OP_DATA;
    int total_bytes = 0;
    int packets_sent = 0;

    write_log("Opening file", LOG_INFO, LOG_FILE);

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) // read until EOF - chunks 
    {

        data_header.payload_size = htonl(bytes_read);
        write(sock, &data_header, sizeof(Header)); //send header
        write(sock, buffer, bytes_read);    //send data

        total_bytes += bytes_read;
        packets_sent++;
    }

    sprintf(log_line, "Total packets sent: %d", packets_sent);
    write_log(log_line, LOG_INFO, LOG_FILE);

    sprintf(log_line, "Total bytes sent: %d", total_bytes);
    write_log(log_line, LOG_INFO, LOG_FILE);

    //log statistics
    write_log("File transfer complete", LOG_INFO, LOG_FILE);

    write_log("Statistics: ", LOG_INFO, LOG_FILE);
    //statistics description
    write_log("RTT -> round-trip time in microseconds", LOG_INFO, LOG_FILE);
    write_log("cwnd-> send congestion window", LOG_INFO, LOG_FILE);
    write_log("ssthresh -> slow start threshold", LOG_INFO, LOG_FILE);
    write_log("retrans -> total retransmissions", LOG_INFO, LOG_FILE);

    collect_tcp_info(sock, log_line, LOG_BUFFER_SIZE);
    write_log(log_line, LOG_INFO, LOG_FILE);

    // Send end of transmission header
    data_header.type = OP_QUIT; // Reusing QUIT to signal end on transmission
    data_header.payload_size = 0;
    write(sock, &data_header, sizeof(Header));

    printf("File '%s' sent.\n", filename);
    fclose(fp);
    return true;
}