
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "./tools/log.h" // lib de logs


// Definições dos tipos de operação do protocolo
#define OP_LIST 0   // comando list
#define OP_PUT 1    // comando put
#define OP_QUIT 2   // comando quit
#define OP_DATA 3   // bloco de dados -- uso interno
#define OP_ERROR 4  // mensagem de erro -- uso interno
#define OP_OK 5     // mensagem ok -- uso interno


#define LOG_FILE_NAME "client.log" // nome do arquivo de log

#define BUFFER_SIZE 1460 // Tamanho típico do MTU TCP
#define FILE_NAME_BUFFER 1024 // Buffer para nomes de arquivos


// Estrutura do cabeçalho para o protocolo da aplicação
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
        fprintf(stderr, "Uso: %s <host> <porta>\n", argv[0]); // Opções para especificar IP e porta do servidor
        write_log("Uso: <programa> <host> <porta>", LOG_ERROR, LOG_FILE);
        return 1;
    }


    // Obtém o PID para nomear o arquivo de log
    pid_t pid = getpid();
    sprintf(LOG_FILE, "client-%d.log", pid);
    sprintf(log_line, "PID do cliente: %d", pid);
    write_log(log_line, LOG_INFO, LOG_FILE);

    const char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int sock_fd;
    struct sockaddr_in server_addr;


    // Cria o socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        perror("Não foi possível criar o socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);


    // Conecta ao servidor
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Falha na conexão");
        close(sock_fd);
        return 1;
    }

    printf("Conectado ao servidor %s:%d\n", server_ip, server_port);


    // Interface de linha de comando
    char command[100], arg[512]; //512 para caminhos longos
    while (1)
    {
        write_log("Comandos disponíveis: list, put <nome_arquivo>, quit\n", LOG_INFO, LOG_FILE);
        write_log("Digite um comando: ", LOG_INFO, LOG_FILE);
        scanf("%s", command); // lê o comando

        Header header;
        if (strcmp(command, "list") == 0) // comando 'list'
        { 
            cmd_list(sock_fd);
        }
        else if (strcmp(command, "put") == 0) // comando 'put'
        { 
            scanf("%s", arg); // lê o argumento (nome do arquivo) do comando put

            // Log do tempo de início do comando PUT
            time_t start_time = time(NULL);
            write_log("Iniciando comando PUT", LOG_INFO, LOG_FILE);
            cmd_put(sock_fd, arg);

            // Log do tempo de término do comando PUT
            time_t end_time = time(NULL);
            double duration = difftime(end_time, start_time);
            sprintf(log_line, "Conexão PUT durou aproximadamente %.2f segundos.", duration);
            write_log(log_line, LOG_INFO, LOG_FILE);
        }
        else if (strcmp(command, "quit") == 0) // comando 'quit'
        { 
            header.type = OP_QUIT;
            header.payload_size = 0;
            write(sock_fd, &header, sizeof(Header));
            break;
        }
        else
        {
            write_log("Comando desconhecido", LOG_WARN, LOG_FILE);
        }
    }

    close(sock_fd);

    write_log("Conexão encerrada", LOG_INFO, LOG_FILE);
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

    // Recebe resposta
    if (read(sock_fd, &header, sizeof(Header)) > 0) //TODO: corrigir para respeitar o MTU
    {
        header.payload_size = ntohl(header.payload_size);
        char *file_list = malloc(header.payload_size + 1);
        read(sock_fd, file_list, header.payload_size);
        file_list[header.payload_size] = '\0';
        printf("--- Arquivos no servidor ---\n%s----------------------------\n", file_list);
        free(file_list);
    }
    return true;
}

bool cmd_put(int sock, const char *filename_full)
{


    // Abre o arquivo (caminho completo) e verifica se existe
    FILE *fp = fopen(filename_full, "rb");
    if (fp == NULL)
    {
        perror("Erro ao abrir arquivo local");
        return false;
    }

    // Separa o nome do arquivo do caminho
    // exemplo: /caminho/para/arquivo.txt -> arquivo.txt
    char *filename = strrchr(filename_full, '/');// Encontra última ocorrência de '/'
    if (filename)
    {
        filename++; // Avança após o '/'
    }
    else
    {
        filename = filename_full; // Sem caminho, usa o nome completo
    }

    write_log("Arquivo aberto com sucesso", LOG_INFO, LOG_FILE);

    sprintf(log_line, "Enviando arquivo: %s", filename);
    write_log(log_line, LOG_INFO, LOG_FILE);

    // envia apenas o nome do arquivo, sem o caminho
    Header put_header;
    put_header.type = OP_PUT;
    put_header.payload_size = htonl(strlen(filename));
    write(sock, &put_header, sizeof(Header));
    write(sock, filename, strlen(filename));


    // Verifica erro do servidor (arquivo duplicado ou outro erro)
    Header response_header;
    if (read(sock, &response_header, sizeof(Header)) > 0) // lê o header de resposta
    {
        if (response_header.type == OP_ERROR)
        {
            response_header.payload_size = ntohl(response_header.payload_size);
            char *err_msg = malloc(response_header.payload_size + 1);
            read(sock, err_msg, response_header.payload_size); // lê a mensagem de erro
            err_msg[response_header.payload_size] = '\0';
            sprintf(log_line, "Erro do servidor: %s", err_msg);
            write_log(log_line, LOG_ERROR, LOG_FILE);
            free(err_msg);
            fclose(fp);
            return false;
        }
        else if (response_header.type == OP_OK)
        {
            write_log("OK para enviar", LOG_INFO, LOG_FILE);
        }
    }
    else
    {
        write_log("Sem resposta do servidor", LOG_ERROR, LOG_FILE);
        fclose(fp);
        return false;
    }


    // Envia o arquivo em blocos
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    Header data_header;
    data_header.type = OP_DATA;
    int total_bytes = 0;
    int packets_sent = 0;

    write_log("Lendo arquivo", LOG_INFO, LOG_FILE);

    time_t start_time = time(NULL);

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) // lê até EOF - em blocos
    {
        data_header.payload_size = htonl(bytes_read);
        write(sock, &data_header, sizeof(Header)); // envia header
        write(sock, buffer, bytes_read);    // envia dados

        total_bytes += bytes_read;
        packets_sent++;
    }

    time_t end_time = time(NULL);
    double duration = difftime(end_time, start_time);
    sprintf(log_line, "Duração da transferência: %.2f segundos", duration);
    write_log(log_line, LOG_INFO, LOG_FILE);

    sprintf(log_line, "Total de pacotes enviados: %d", packets_sent);
    write_log(log_line, LOG_INFO, LOG_FILE);

    sprintf(log_line, "Total de bytes enviados: %d", total_bytes);
    write_log(log_line, LOG_INFO, LOG_FILE);

    // log de estatísticas
    write_log("Transferência de arquivo concluída", LOG_INFO, LOG_FILE);

    write_log("Estatísticas: ", LOG_INFO, LOG_FILE);
    // descrição das estatísticas
    write_log("RTT -> tempo de ida e volta em microssegundos", LOG_INFO, LOG_FILE);
    write_log("cwnd -> janela de congestionamento de envio", LOG_INFO, LOG_FILE);
    write_log("ssthresh -> limiar de slow start", LOG_INFO, LOG_FILE);
    write_log("retrans -> total de retransmissões", LOG_INFO, LOG_FILE);

    collect_tcp_info(sock, log_line, LOG_BUFFER_SIZE);
    write_log(log_line, LOG_INFO, LOG_FILE);

    // taxa aproximada
    if (duration > 0) {
        double rate = total_bytes / duration; // bytes por segundo
        sprintf(log_line, "Taxa aproximada de transferência: %.2f bytes/segundo", rate);
        write_log(log_line, LOG_INFO, LOG_FILE);
    }
    // Envia header de fim de transmissão
    data_header.type = OP_QUIT; // Reutiliza QUIT para sinalizar fim
    data_header.payload_size = 0;
    write(sock, &data_header, sizeof(Header));

    printf("Arquivo '%s' enviado.\n", filename);
    fclose(fp);
    return true;
}