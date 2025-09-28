// servidor.c
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



// Definição dos tipos de operação do protocolo [cite: 23]
#define OP_LIST 0
#define OP_PUT 1
#define OP_QUIT 2
#define OP_DATA 3
#define OP_ERROR 4
#define OP_OK 5


#define STORAGE_DIR "storage/"

#define BUFFER_SIZE 1460

#define FILE_PATH "received/"
#define FILE_NAME "data.txt"
#define LOG_FILE "server.log"

char log_line[LOG_BUFFER_SIZE] = {0};


// Estrutura do cabeçalho do nosso protocolo de aplicação
#pragma pack(1)
typedef struct {
    char type;
    uint32_t payload_size;
} Header;
#pragma pack()



void *handle_client(void *socket_desc);
void send_file_list(int sock);
void receive_file(int sock, char* filename);
void exit_error(char* buff);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <porta>\n", argv[0]);
        return 1;
    }

    // Cria o diretório de armazenamento se não existir [cite: 30]
    mkdir(STORAGE_DIR, 0777);

    int server_fd, client_sock, c;
    struct sockaddr_in server, client;

    // Criar socket TCP/IPv4 [cite: 8]
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {

        write_log("Socket creation failed", LOG_ERROR, LOG_FILE);
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(argv[1]));

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        close(server_fd);
        exit_error("Bind failed");
    }

    // Listen
    listen(server_fd, 5); // Fila de até 5 conexões pendentes


    sprintf(log_line, "Server started on port %s", argv[1]);
    write_log(log_line, LOG_INFO, LOG_FILE);
    write_log("Waiting for connections...", LOG_INFO, LOG_FILE);

    //show licalip
    char local_ip[INET_ADDRSTRLEN];
    struct sockaddr_in sa;
    socklen_t sa_len = sizeof(sa);
    getsockname(server_fd, (struct sockaddr *)&sa, &sa_len);
    inet_ntop(AF_INET, &sa.sin_addr, local_ip, sizeof(local_ip));
    sprintf(log_line, "Server IP: %s", local_ip);
    write_log(log_line, LOG_INFO, LOG_FILE);


     write_log("step 1", LOG_INFO, LOG_FILE);
    c = sizeof(struct sockaddr_in);
      write_log("step 2", LOG_INFO, LOG_FILE);
    while ((client_sock = accept(server_fd, (struct sockaddr *)&client, (socklen_t*)&c))) {
          write_log("step 1", LOG_INFO, LOG_FILE);
       // sprintf(log_line, "Connection accepted from %s:%d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        //write_log(log_line, LOG_INFO, LOG_FILE);

        pthread_t thread_id;
        int *new_sock = malloc(1);
        *new_sock = client_sock;

        // Cria uma nova thread para cada cliente (concorrência) [cite: 11]
        if (pthread_create(&thread_id, NULL, handle_client, (void*) new_sock) < 0) {
            free(new_sock);
            close(client_sock);
            exit_error("Could not create thread");
            continue;
        }

        // A thread principal continua a esperar por novas conexões
    }

    if (client_sock < 0) {
        write_log("Accept failed", LOG_ERROR, LOG_FILE);
        close(server_fd);
        return 1;
    }

    close(server_fd);
    return 0;
}

void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    Header header;
    char buffer[1024];

    while (read(sock, &header, sizeof(Header)) > 0) {
        header.payload_size = ntohl(header.payload_size);
        
        // Lê o payload se houver
        if (header.payload_size > 0) {
            read(sock, buffer, header.payload_size);
            buffer[header.payload_size] = '\0';
        }

        switch (header.type) {
            case OP_LIST: // Operação LIST 
              
                write_log("Cliente solicitou a lista de arquivos.", LOG_INFO, LOG_FILE);
                send_file_list(sock);
                break;
            case OP_PUT: // Operação PUT
                sprintf(log_line, "Cliente quer enviar o arquivo: %s", buffer);
                write_log(log_line, LOG_INFO, LOG_FILE);
                receive_file(sock, buffer);
                break;
            case OP_QUIT: // Operação QUIT
                sprintf(log_line, "Cliente solicitou encerramento da sessão.");
                write_log(log_line, LOG_INFO, LOG_FILE);
                close(sock);
                return 0;
            default:
                sprintf(log_line, "Operação desconhecida: %d", header.type);
                write_log(log_line, LOG_ERROR, LOG_FILE);
                break;
        }
    }

    write_log("Cliente desconectado.", LOG_INFO, LOG_FILE);
    close(sock);
    return 0;
}

void send_file_list(int sock) {
    DIR *d;
    struct dirent *dir;
    char file_list[4096] = "";

    d = opendir(STORAGE_DIR);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
           // if (dir->d_type == DT_REG) { // Apenas arquivos regulares
                strcat(file_list, dir->d_name);
                strcat(file_list, "\n");
            //}
        }
        closedir(d);
    }

    Header response_header;
    response_header.type = OP_LIST;
    response_header.payload_size = htonl(strlen(file_list));

    write(sock, &response_header, sizeof(Header));
    write(sock, file_list, strlen(file_list));
}

void receive_file(int sock, char* filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", STORAGE_DIR, filename);
    write_log("Receiving file...", LOG_INFO, LOG_FILE);
    write_log(filepath, LOG_INFO, LOG_FILE);
    // Tratamento de erro para arquivos com o mesmo nome
    if (access(filepath, F_OK) == 0) {
        write_log("File already exists on server.", LOG_ERROR, LOG_FILE);
        // File already exists
        sprintf(log_line,"file name: %s", filename);
        write_log(filename, LOG_ERROR, LOG_FILE);
        Header err_header;
        err_header.type = OP_ERROR;
        char *msg = "Arquivo com este nome ja existe.";
        err_header.payload_size = htonl(strlen(msg));
        write(sock, &err_header, sizeof(Header));
        write(sock, msg, strlen(msg));
        return;
    }

    write_log("File does not exist, ready to receive.", LOG_INFO, LOG_FILE);
    // Header ok_header;
    // ok_header.type = OP_OK;
    // char *msg = "Arquivo com este nome ja existe.";
    // ok_header.payload_size = 0;

    // write(sock, &ok_header, sizeof(Header));
    // write(sock, msg, strlen(msg));

    sprintf(log_line, "file name: %s", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);

    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL) {
        write_log("Erro ao criar arquivo no servidor.", LOG_ERROR, LOG_FILE);
        return;
    }
    write_log("file created!", LOG_INFO, LOG_FILE);


    //send confirmation to client
    Header ok_header;
    ok_header.type = OP_OK;
    ok_header.payload_size = 0;
    write(sock, &ok_header, sizeof(Header));    
    
     



    Header data_header;
    char data_buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    
    // Loop para receber os dados do arquivo
    while ((bytes_read = read(sock, &data_header, sizeof(Header))) > 0) {
         write_log("block received", LOG_INFO, LOG_FILE);
        if (data_header.type != OP_DATA) {
            break; // Fim da transmissão do arquivo
        }
        data_header.payload_size = ntohl(data_header.payload_size);
        
        read(sock, data_buffer, data_header.payload_size);
        fwrite(data_buffer, 1, data_header.payload_size, fp);
    }

    sprintf(log_line, "File '%s' received successfully.", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);
    fclose(fp);
}

void exit_error(char* buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}
