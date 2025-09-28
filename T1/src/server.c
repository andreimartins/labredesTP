// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <dirent.h>

#include "./tools/log.h"

// Protocol operation type definitions
#define OP_LIST 0   //cmd list
#define OP_PUT 1    //cmd put
#define OP_QUIT 2   //cmd quit
#define OP_DATA 3   //data block -- internal use
#define OP_ERROR 4  //error message -- internal use
#define OP_OK 5     //ok message -- internal use

#define STORAGE_DIR "storage/" // Directory to store received files

#define BUFFER_SIZE 1460  // TCP typical MTU size
#define LOG_FILE "server.log" //log file name

char log_line[LOG_BUFFER_SIZE] = {0};

// Header structure for our application protocol
#pragma pack(1)
typedef struct
{
    char type;
    uint32_t payload_size;
} Header;
#pragma pack() // pack is used to avoid compiler adding padding bytes

void *handle_client(void *socket_desc);
void send_file_list(int sock);
void receive_file(int sock, char *filename);
void exit_error(char *buff);

int main(int argc, char *argv[])
{
    char client_ip[INET_ADDRSTRLEN];
    char server_ip_true[INET_ADDRSTRLEN] = "0.0.0.0";

    int server_fd, client_sock, c_len;
    struct sockaddr_in server, client;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    // Create storage directory if it does not exist
    mkdir(STORAGE_DIR, 0764); // 0764 = rwx rw- r-- permissions 

    //log ip address from server
    FILE *f = popen("hostname -I", "r"); //command to get IP address
    if (f && fgets(server_ip_true, sizeof(server_ip_true), f)) { // parse the output to get the first block (IP)
        char *nl = strchr(server_ip_true, ' ');
        if (nl) *nl = '\0';
        pclose(f);
    }
    sprintf(log_line, "Server IP: %s", server_ip_true);
    write_log(log_line, LOG_INFO, LOG_FILE);



    // Initialize socket
    // Create TCP/IPv4 socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {

        write_log("Socket creation failed", LOG_ERROR, LOG_FILE);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // any interface
    server.sin_port = htons(atoi(argv[1]));

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        close(server_fd);
        exit_error("Bind failed");
    }

    // Listen
    listen(server_fd, 5); // Queue up to 5 pending connections

    sprintf(log_line, "Server started on port %s", argv[1]);
    write_log(log_line, LOG_INFO, LOG_FILE);
    write_log("Waiting for connections...", LOG_INFO, LOG_FILE);

    // // Show local IP -- 0.0.0.0 to any interface
    // char server_ip [INET_ADDRSTRLEN];
    // struct sockaddr_in sa;
    // socklen_t sa_len = sizeof(sa);
    // getsockname(server_fd, (struct sockaddr *)&sa, &sa_len);
    // inet_ntop(AF_INET, &sa.sin_addr, server_ip, sizeof(server_ip));
    // sprintf(log_line, "Server IP: %s", server_ip);
    // write_log(log_line, LOG_INFO, LOG_FILE);


    c_len = sizeof(struct sockaddr_in);

    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&c_len)))
    {
        //log client address


        inet_ntop(AF_INET, &(client.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client.sin_port);

        sprintf(log_line, "Connection accepted from %s:%d", client_ip, client_port);
        write_log(log_line, LOG_INFO, LOG_FILE);

        pthread_t thread_id;
        int *new_sock = malloc(1);
        *new_sock = client_sock;

        // Create a new thread for each client (concurrency)
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) < 0)
        {
            free(new_sock);
            close(client_sock);
            exit_error("Could not create thread");
            continue;
        }

        // Main thread continues to wait for new connections
    }

    if (client_sock < 0)
    {
        write_log("Accept failed", LOG_ERROR, LOG_FILE);
        close(server_fd);
        return 1;
    }

    close(server_fd);
    return 0;
}

void *handle_client(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    free(socket_desc);
    Header header;
    char buffer[1024];

    while (read(sock, &header, sizeof(Header)) > 0)
    {
        header.payload_size = ntohl(header.payload_size);

        // Lê o payload se houver
        if (header.payload_size > 0)
        {
            read(sock, buffer, header.payload_size);
            buffer[header.payload_size] = '\0';
        }

        switch (header.type)
        {
        case OP_LIST: // LIST operation
            write_log("Client requested file list.", LOG_INFO, LOG_FILE);
            send_file_list(sock);
            break;
        case OP_PUT: // PUT operation
            sprintf(log_line, "Client wants to send file: %s", buffer);
            write_log(log_line, LOG_INFO, LOG_FILE);
            receive_file(sock, buffer);
            break;
        case OP_QUIT: // QUIT operation
            sprintf(log_line, "Client requested session termination.");
            write_log(log_line, LOG_INFO, LOG_FILE);
            close(sock);
            return 0;
        default:
            sprintf(log_line, "Unknown operation: %d", header.type);
            write_log(log_line, LOG_ERROR, LOG_FILE);
            break;
        }
    }

    write_log("Client disconnected.", LOG_INFO, LOG_FILE);
    close(sock);
    return 0;
}

void send_file_list(int sock)
{
    DIR *d;
    struct dirent *dir;
    char file_list[4096] = "";

    d = opendir(STORAGE_DIR);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            // Only regular files
            strcat(file_list, dir->d_name);
            strcat(file_list, "\n");
        }
        closedir(d);
    }

    Header response_header;
    response_header.type = OP_LIST;
    response_header.payload_size = htonl(strlen(file_list));

    write(sock, &response_header, sizeof(Header));
    write(sock, file_list, strlen(file_list));
}

void receive_file(int sock, char *filename)
{
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", STORAGE_DIR, filename);
    write_log("Receiving file...", LOG_INFO, LOG_FILE);
    write_log(filepath, LOG_INFO, LOG_FILE);

    // Error handling for files with the same name
    if (access(filepath, F_OK) == 0)
    {
        write_log("File already exists on server.", LOG_ERROR, LOG_FILE);
        Header err_header;
        err_header.type = OP_ERROR;
        char *msg = "A file with this name already exists.";
        err_header.payload_size = htonl(strlen(msg));
        write(sock, &err_header, sizeof(Header));
        write(sock, msg, strlen(msg));
        return;
    }

    write_log("File does not exist, ready to receive.", LOG_INFO, LOG_FILE);

    sprintf(log_line, "file name: %s", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);

    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL)
    {
        write_log("Error creating file on server.", LOG_ERROR, LOG_FILE);
        return;
    }
    write_log("File created!", LOG_INFO, LOG_FILE);

    // Send confirmation to client
    Header ok_header;
    ok_header.type = OP_OK;
    ok_header.payload_size = 0;
    write(sock, &ok_header, sizeof(Header)); //send only ok without message

    Header data_header;
    char data_buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int total_blocks = 0;
    // Loop to receive file data
    while ((bytes_read = read(sock, &data_header, sizeof(Header))) > 0) //read header
    {

        if (data_header.type != OP_DATA)
        {
            break; // End of file transmission
        }
        data_header.payload_size = ntohl(data_header.payload_size);
        read(sock, data_buffer, data_header.payload_size);
        fwrite(data_buffer, 1, data_header.payload_size, fp);
    }

    sprintf(log_line, "File '%s' received successfully.", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);
    fclose(fp);
}

void exit_error(char *buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}
