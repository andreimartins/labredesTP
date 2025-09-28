
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "./tools/log.h"

// Definição dos tipos de operação do protocolo
#define OP_LIST 0
#define OP_PUT 1
#define OP_QUIT 2
#define OP_DATA 3
#define OP_ERROR 4
#define OP_OK 5

#define FILE_PATH "received/"

#define LOG_FILE "client.log"


#define BUFFER_SIZE 1460
#define FILE_NAME_BUFFER 1024


// Estrutura do cabeçalho do nosso protocolo de aplicação
#pragma pack(1)
typedef struct {
    char type;
    uint32_t payload_size;
} Header;
#pragma pack()



//log line buffer
char log_line[LOG_BUFFER_SIZE] = {0};
void exit_error(char* buff);
bool cmd_list(int sock_fd);
bool cmd_put(int sock, const char* filename);

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <host> <porta>\n", argv[0]); // Opções para especificar servidor 
        write_log("Uso: <programa> <host> <porta>", LOG_ERROR, LOG_FILE);
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock_fd;
    struct sockaddr_in server_addr;

    // Criar socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("Não foi possível criar o socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Conectar ao servidor
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Conexão falhou");
        close(sock_fd);
        return 1;
    }

    printf("Conectado ao servidor %s:%d\n", server_ip, server_port);

    // Implementação da interface de linha de comando [cite: 32]
    char command[100], arg[512];
    while (1) {
        
       
        write_log("Comandos disponíveis: list, put <filename>, quit\n", LOG_INFO, LOG_FILE);
        write_log("Digite um comando: ", LOG_INFO, LOG_FILE);
        scanf("%s", command);

        Header header;
        if (strcmp(command, "list") == 0) { // Comando 'list' [cite: 36]

            cmd_list(sock_fd);

        } else if (strcmp(command, "put") == 0) { // Comando 'put' [cite: 34]
            scanf("%s", arg);
            
            // Criação de log por conexão (exemplo simples) [cite: 45]
            time_t start_time = time(NULL);
            write_log("Starting PUT command", LOG_INFO, LOG_FILE);
            cmd_put(sock_fd, arg);

            time_t end_time = time(NULL);
            double duration = difftime(end_time, start_time);
            sprintf(log_line, "Connection for PUT lasted approximately %.2f seconds.", duration);
            write_log(log_line, LOG_INFO, LOG_FILE);

        } else if (strcmp(command, "quit") == 0) { // Comando 'quit' [cite: 39]
            header.type = OP_QUIT;
            header.payload_size = 0;
            write(sock_fd, &header, sizeof(Header));
            break;
        } else {
          
            write_log("Comando desconhecido", LOG_WARN, LOG_FILE);
        }
    }

    close(sock_fd);

    write_log("Connection closed", LOG_INFO, LOG_FILE);
    return 0;
}


void exit_error(char* buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}

bool cmd_list(int sock_fd) {

    Header header;
    header.type = OP_LIST;
    header.payload_size = 0;
    write(sock_fd, &header, sizeof(Header));
    
    // Receber resposta
    if (read(sock_fd, &header, sizeof(Header)) > 0) {
        header.payload_size = ntohl(header.payload_size);
        char *file_list = malloc(header.payload_size + 1);
        read(sock_fd, file_list, header.payload_size);
        file_list[header.payload_size] = '\0';
        printf("--- Files on server ---\n%s----------------------------\n", file_list);
        free(file_list);
    }
    return true;
}

bool cmd_put(int sock, const char* filename_full) {
    

    //openfile full path and check if exists

        FILE *fp = fopen(filename_full, "rb");
    if (fp == NULL) {
        perror("Erro ao abrir arquivo local");
        return false;
    }

    // split the filename from the path
    char *filename = strrchr(filename_full, '/');
    if (filename) {
        filename++;  // Move past the '/'
    } else {
        filename = filename_full;  // No path, use the whole filename
    }

    write_log("Opened file successfully", LOG_INFO, LOG_FILE);

    sprintf(log_line, "Sending file: %s", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);

    Header put_header;
    put_header.type = OP_PUT;
    put_header.payload_size = htonl(strlen(filename));
    write(sock, &put_header, sizeof(Header));
    write(sock, filename, strlen(filename));

    // Verificar se há erro do servidor (arquivo duplicado)
    Header response_header;
    if (read(sock, &response_header, sizeof(Header)) > 0) {
        if (response_header.type == OP_ERROR) {
            response_header.payload_size = ntohl(response_header.payload_size);
            char *err_msg = malloc(response_header.payload_size + 1);
            read(sock, err_msg, response_header.payload_size);
            err_msg[response_header.payload_size] = '\0';
            //fprintf(stderr, "Erro do servidor: %s\n", err_msg);
            sprintf(log_line, "Server error: %s", err_msg);
            write_log(log_line, LOG_ERROR, LOG_FILE);
            free(err_msg);
            fclose(fp);
            return false;
        }
        else if(response_header.type == OP_OK)
        {
            write_log("OK to send", LOG_INFO, LOG_FILE);
        }

        // Se não for erro, o servidor está pronto. O primeiro read foi consumido,
        // mas precisamos tratar esse dado (assumindo que era um ACK implícito).
        // Para uma implementação robusta, um ACK explícito seria melhor.
    }
    // Se chegou aqui, o servidor está pronto para receber o arquivo
    write_log("Server ready to receive file", LOG_INFO, LOG_FILE);

    // Enviar o arquivo em blocos

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    Header data_header;
    data_header.type = OP_DATA;
    int total_bytes = 0;
    int packets_sent = 0;

    write_log("Openning file", LOG_INFO, LOG_FILE );

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        write_log("block readed", LOG_INFO,LOG_FILE);

        data_header.payload_size = htonl(bytes_read);
        write(sock, &data_header, sizeof(Header));
        write(sock, buffer, bytes_read);

        total_bytes += bytes_read;
        packets_sent ++;
    }

    sprintf(log_line, "Total packets sent: %d", packets_sent);
    write_log(log_line, LOG_INFO, LOG_FILE);

    sprintf(log_line, "Total bytes sent: %d", total_bytes);
    write_log(log_line, LOG_INFO, LOG_FILE);

    write_log("Statistics: ", LOG_INFO, LOG_FILE);
    collect_tcp_info(sock, log_line, LOG_BUFFER_SIZE);
    write_log(log_line, LOG_INFO, LOG_FILE);

    // Enviar cabeçalho de fim de transmissão
    data_header.type = OP_QUIT; // Reutilizando QUIT para sinalizar fim
    data_header.payload_size = 0;
    write(sock, &data_header, sizeof(Header));
    
    printf("File '%s' sent.\n", filename);
    fclose(fp);
    return true;
}