#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "./tools/log.h"

#define FILE_PATH "received/"
#define FILE_NAME "data.txt"


#define LOG_FILE "client.log"


#define BUFFER_SIZE 1460

//log line buffer
char log_line[LOG_BUFFER_SIZE] = {0};
void exit_error(char* buff);
bool cmd_list(const char* command);
bool cmd_put(char* file_to_send, int sock);




int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;

    //verify if all parameters are passed
    if (argc != 4) {
        sprintf(log_line, "Usage: %s <server_ip> <server_port> <file_selection 1-small 2-large>\n", argv[0]);
        write_log(log_line, LOG_ERROR, LOG_FILE);
        exit_error("Invalid arguments");
    }

    write_log("Client started", LOG_INFO, LOG_FILE);
    char cmd[150] = {0};
    char server_ip[16] = {0};
    char server_port[6] = {0};
    int cmd_selection = 0;

    // load the parameters
    sprintf(server_ip, "%s", argv[1]);
    sprintf(server_port, "%s", argv[2]);
    //file_selection = atoi(argv[3]);
    int port_number = atoi(server_port);

    sprintf(log_line, "Connecting to server %s:%s", server_ip, server_port);
    write_log(log_line, LOG_INFO, LOG_FILE);

    
    // create the socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        

        exit_error("Socket creation error");
    }

    // set the server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        

        close(sock);
        exit_error("Invalid address/ Address not supported");
    }

    // connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {

        close(sock);
        exit_error("Connection failed");
    }
   
    sprintf(log_line, "Connected to server %s:%d", server_ip, port_number);
    write_log(log_line, LOG_INFO, LOG_FILE);

    while(cmd_selection != 3) // while not quit
    {

        printf("Digite o comando (list, put ou quit): ");
        scanf("%49s", cmd);

        if (strcmp(cmd, "list") == 0) {
            cmd_selection = 1; 
        } else if (strcmp(cmd, "put") == 0) {
            cmd_selection = 2; 
        }
        else if (strcmp(cmd, "quit") == 0) {
            cmd_selection = 3; 
        }
        else {
            printf("Comando inválido.\n");
            return 1;
        }
        memset(cmd, 0, sizeof(cmd));// clear cmd buffer

        // open the selected file
        switch(cmd_selection) {
            case 1:
                    if(cmd_list("list")) 
                    {
                        write_log("List command executed", LOG_INFO, LOG_FILE);
                    } 
                    else 
                    {
                        write_log("List command failed", LOG_ERROR, LOG_FILE);
                    }

                break;
            case 2:     
                    printf("informe o caminho e o arquivo a ser enviado: ");
                    printf("ex.: livro.txt \n");
                    printf("ex.: /home/user/Documentos/livro.txt \n");

                    scanf("%49s", cmd);


                    sprintf(log_line, "Sending file %s", cmd);
                    write_log(log_line, LOG_INFO, LOG_FILE);


                    if(cmd_put (cmd, sock)) 
                    {
                        write_log("Put command executed", LOG_INFO, LOG_FILE);
                    } 
                    else 
                    {
                        write_log("Put command failed", LOG_ERROR, LOG_FILE);
                    }

                break;
            case 3:     
                    sprintf(log_line, "Quitting...");
                    write_log(log_line, LOG_INFO, LOG_FILE);
                break;
            default:
                exit_error("Invalid file selection");
                break;

        }
    }

   
    
    // close the socket
    close(sock);

    return 0;
}

void exit_error(char* buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}

bool cmd_list(const char* command) {
    //todo: envia estrutura de list e aguarda resposta do servidor
    return true;
}

bool cmd_put(char* file_to_send, int sock) {
    char buffer[BUFFER_SIZE] = {0};
    size_t bytes_read;
    
    if (!verify_file_exists(file_to_send)) {
        write_log("File does not exist", LOG_ERROR, LOG_FILE);
        return false;
    }

    FILE *file = fopen(file_to_send, "rb");

        
       //if fails to open the file to send, exit
    if (file == NULL) {
        
        write_log(log_line, LOG_ERROR, LOG_FILE);
        exit_error("Failed to open file");
    }


    // read the file to send in blocks
    int packets_sent = 0;
    int total_bytes = 0;
    while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
        if (send(sock, buffer, bytes_read, 0) == -1) {
            perror("Failed to send file");
            write_log("Failed to send file", LOG_ERROR, LOG_FILE);
            fclose(file);
           // close(sock);
           // exit(EXIT_FAILURE);
            return false;
        }
        //sprintf(log_line,"Sent %zu bytes", bytes_read);
        //write_log(log_line, LOG_INFO, LOG_FILE);
        memset(buffer, 0, BUFFER_SIZE); // Limpar o buffer
        packets_sent++;
        total_bytes += bytes_read;
        collect_tcp_info(sock, log_line, LOG_BUFFER_SIZE);
        write_log(log_line, LOG_INFO, LOG_FILE);
    }

    sprintf(log_line, "Total packets sent: %d", packets_sent);
    write_log(log_line, LOG_INFO, LOG_FILE);
    sprintf(log_line, "Total bytes sent: %d", total_bytes);
    write_log(log_line, LOG_INFO, LOG_FILE);

    collect_tcp_info(sock, log_line, LOG_BUFFER_SIZE);
    write_log(log_line, LOG_INFO, LOG_FILE);
    fclose(file);
    return true;
}