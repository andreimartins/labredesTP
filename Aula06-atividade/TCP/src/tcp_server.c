#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h> //to list files in a directory

#include "./tools/log.h"
#include "./tools/Utils.h"



#define PORT 15000
#define BUFFER_SIZE 1460

#define FILE_PATH "received/"
#define FILE_NAME "data.txt"
#define LOG_FILE "server.log"

char log_line[LOG_BUFFER_SIZE] = {0};



int write_file(char* data);
void exit_error(char* buff);
void clear_file();


int main(int argc, char *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char ip_address[INET_ADDRSTRLEN] = {0}; // Buffer para armazenar o IP

    char port[6] = {0};
    sprintf(ip_address, "%s", argv[1]);
    sprintf(port, "%s", argv[2]);
    int port_number = atoi(port);
   

    write_log("Server started", LOG_INFO, LOG_FILE);
    // Ensure the directory exists



    char filelist[4096] = {0};
    verify_dir(FILE_PATH);
    list_files(FILE_PATH, filelist);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {

        exit_error("Socket failed");
    }

    // Bind the socket to the network address and port
    if (inet_pton(AF_INET, ip_address, &address.sin_addr) <= 0) {
        close(server_fd);
        exit_error("Invalid address/ Address not supported");
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(port_number);

    //bind socket address
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        exit_error("Bind failed");
        
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        close(server_fd);
        exit_error("Listen failed");
        

    }
    write_log("Server is listening for connections", LOG_INFO, LOG_FILE);
    // Log the address and port
    sprintf(log_line, "Server listening on address: %s port: %d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
    
    write_log(log_line, LOG_INFO, LOG_FILE);
    

    // Accept connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        close(server_fd);
        exit_error("Accept failed");
    }
    


    // Receive data in a loop and write to file
    int valread;
    int counter = 0;
    while ((valread = read(new_socket, buffer, BUFFER_SIZE)) > 0) 
    {
        //sprintf(log_line, "Received %d bytes", valread);
        //write_log(log_line, LOG_INFO, LOG_FILE);
        write_file(buffer);
        memset(buffer, 0, valread); // Clear the buffer
        counter++;
    

    }
    sprintf(log_line, "Total packets received: %d", counter);
    write_log(log_line, LOG_INFO, LOG_FILE);

    write_log("File received and saved successfully", LOG_INFO, LOG_FILE);

    // Close file and sockets
    close(new_socket);
    close(server_fd);

    return 0;
}


void exit_error(char* buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}


int write_file(char* data)
{
    FILE *file;

    char data_recv_path[sizeof(FILE_PATH) + sizeof(FILE_NAME)];
    sprintf(data_recv_path, "%s%s", FILE_PATH, FILE_NAME);
   


    if (access(data_recv_path, F_OK) == 0) {
        file = fopen(data_recv_path, "a");
     
    } else {
        file = fopen(data_recv_path, "w");
    }

    if (file == NULL) {
        exit_error("Failed to open file");
    }

    // Write data to the file
    fprintf(file, data);

    // Close the file
    fclose(file);
    return 0;
}

void list_files(const char *path, char* file_list) {
    struct dirent *entry;
    DIR *dp = opendir(path);

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    sprintf(log_line, "Files: \n");
    write_log(log_line, LOG_INFO, LOG_FILE);
    while ((entry = readdir(dp))) {
        sprintf(log_line, "%s\n", entry->d_name);
        write_log(log_line, LOG_INFO, LOG_FILE);
    }

    closedir(dp);
}