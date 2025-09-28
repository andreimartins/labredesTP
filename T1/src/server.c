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

#include "./tools/log.h" //lib de logs


// Definições dos tipos de operação do protocolo
#define OP_LIST 0   // comando list
#define OP_PUT 1    // comando put
#define OP_QUIT 2   // comando quit
#define OP_DATA 3   // bloco de dados -- uso interno
#define OP_ERROR 4  // mensagem de erro -- uso interno
#define OP_OK 5     // mensagem ok -- uso interno

#define STORAGE_DIR "storage/" // Diretório para armazenar arquivos recebidos

#define BUFFER_SIZE 1460  // Tamanho típico do MTU TCP
#define LOG_FILE "server.log" // nome do arquivo de log

char log_line[LOG_BUFFER_SIZE] = {0};


// Estrutura do cabeçalho para o protocolo da aplicação
#pragma pack(1)
typedef struct
{
    char type;
    uint32_t payload_size;
} Header;
#pragma pack() // pack é usado para evitar que o compilador adicione bytes de preenchimento

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
        fprintf(stderr, "Uso: %s <porta>\n", argv[0]);
        return 1;
    }


    // Cria o diretório de armazenamento se não existir
    mkdir(STORAGE_DIR, 0764); // 0764 = permissões rwx rw- r--


    // Log do endereço IP do servidor
    FILE *f = popen("hostname -I", "r"); // comando para obter o endereço IP
    if (f && fgets(server_ip_true, sizeof(server_ip_true), f)) {
        char *nl = strchr(server_ip_true, ' ');
        if (nl) *nl = '\0';
        pclose(f);
    }
    sprintf(log_line, "IP do servidor: %s", server_ip_true);
    write_log(log_line, LOG_INFO, LOG_FILE);




    // Inicializa o socket
    // Cria socket TCP/IPv4
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        write_log("Falha na criação do socket", LOG_ERROR, LOG_FILE);
        return 1;
    }


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // qualquer interface
    server.sin_port = htons(atoi(argv[1]));


    // Faz o bind
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        close(server_fd);
        exit_error("Falha no bind");
    }


    // Escuta
    listen(server_fd, 5); // Fila de até 5 conexões pendentes

    sprintf(log_line, "Servidor iniciado na porta %s", argv[1]);
    write_log(log_line, LOG_INFO, LOG_FILE);
    write_log("Aguardando conexões...", LOG_INFO, LOG_FILE);

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
        // Log do endereço do cliente
        inet_ntop(AF_INET, &(client.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client.sin_port);

        sprintf(log_line, "Conexão aceita de %s:%d", client_ip, client_port);
        write_log(log_line, LOG_INFO, LOG_FILE);

        pthread_t thread_id;
        int *new_sock = malloc(1);
        *new_sock = client_sock;

        // Cria uma nova thread para cada cliente (concorrência)
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_sock) < 0)
        {
            free(new_sock);
            close(client_sock);
            exit_error("Não foi possível criar thread");
            continue;
        }

        // Thread principal continua aguardando novas conexões
    }


    if (client_sock < 0)
    {
        write_log("Falha no accept", LOG_ERROR, LOG_FILE);
        close(server_fd);
        return 1;
    }

    close(server_fd);
    return 0;
}

// função que será executada na thread que lidará com o client
void *handle_client(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    free(socket_desc);
    Header header;
    char buffer[1024];

    while (read(sock, &header, sizeof(Header)) > 0) //laço para recebimento de comandos
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
        case OP_LIST: // operação LIST
            write_log("Cliente solicitou lista de arquivos.", LOG_INFO, LOG_FILE);
            send_file_list(sock);
            break;
        case OP_PUT: // operação PUT
            sprintf(log_line, "Cliente quer enviar arquivo: %s", buffer);
            write_log(log_line, LOG_INFO, LOG_FILE);
            receive_file(sock, buffer);
            break;
        case OP_QUIT: // operação QUIT
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

//execução do comando de list 
void send_file_list(int sock)
{
    DIR *d;
    struct dirent *dir;
    char file_list[4096] = "";

    d = opendir(STORAGE_DIR); // abre o diretório storage
    if (d)
    {
        while ((dir = readdir(d)) != NULL) // lê lista de arquivos 
        {
            // Apenas arquivos regulares
            
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) { //ingora . e ..
                continue;
            }
            strcat(file_list, dir->d_name); //montagem da lista de nomes de arquivos
            strcat(file_list, "\n");
        }
        closedir(d); // fecha o diretório
    }

    Header response_header;
    response_header.type = OP_LIST;
    response_header.payload_size = htonl(strlen(file_list));

    //TODO: corrigir para respeitar o MTU
    write(sock, &response_header, sizeof(Header)); // envia header informando que ira enviar a lista 
    write(sock, file_list, strlen(file_list)); // envia a lista 
}


//recebimento de arquivo enviado pelo client
void receive_file(int sock, char *filename)
{
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s%s", STORAGE_DIR, filename);
    write_log("Recebendo arquivo...", LOG_INFO, LOG_FILE);
    write_log(filepath, LOG_INFO, LOG_FILE);

    // verifica se o arquivo já existe, caso já exista retorna erro para informar duplicidade
    if (access(filepath, F_OK) == 0)
    {
        write_log("Arquivo já existe no servidor.", LOG_ERROR, LOG_FILE);
        Header err_header;
        err_header.type = OP_ERROR;
        char *msg = "Já existe um arquivo com esse nome.";
        err_header.payload_size = htonl(strlen(msg));
        write(sock, &err_header, sizeof(Header));
        write(sock, msg, strlen(msg));
        return;
    }

    write_log("Arquivo não existe, pronto para receber.", LOG_INFO, LOG_FILE);

    sprintf(log_line, "nome do arquivo: %s", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);

    //cria arquivo a ser recebido
    FILE *fp = fopen(filepath, "wb");
    if (fp == NULL)
    {
        write_log("Erro ao criar arquivo no servidor.", LOG_ERROR, LOG_FILE);
        return;
    }
    write_log("Arquivo criado!", LOG_INFO, LOG_FILE);

    // Envia confirmação para o cliente
    Header ok_header;
    ok_header.type = OP_OK;
    ok_header.payload_size = 0;
    write(sock, &ok_header, sizeof(Header)); // envia apenas ok sem mensagem descritiva - controle interno

    //  fluco de recebimento
    Header data_header;
    char data_buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int total_blocks = 0;
    // Loop para receber dados do arquivo
    while ((bytes_read = read(sock, &data_header, sizeof(Header))) > 0) // recebe o header da msg
    {
        if (data_header.type != OP_DATA) //qualquer mensagem diferente de OP_DATA encerra o recebimento
        {
            break; // Fim da transmissão do arquivo
        }
        data_header.payload_size = ntohl(data_header.payload_size); 
        read(sock, data_buffer, data_header.payload_size);      // recebe o dado
        fwrite(data_buffer, 1, data_header.payload_size, fp);   // escreve no arquivo parte do dado recebido
    }

    sprintf(log_line, "Arquivo '%s' recebido com sucesso.", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);
    fclose(fp);
}

void exit_error(char *buff)
{
    perror(buff);
    write_log(buff, LOG_INFO, LOG_FILE);
    exit(EXIT_FAILURE);
}
