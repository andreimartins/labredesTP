// main.c
#include "capture.h"
#include "analyze.h"
#include "log.h"
#include <unistd.h>
#include <signal.h>
#include "stats.h" 
#include <unistd.h> // Para sleep (ou nanosleep)
// Buffer para o pacote bruto
unsigned char packet_buffer[BUFFER_SIZE];
// Estrutura para informações extraídas
PacketInfo packet_info;
int raw_socket_fd = -1;
volatile sig_atomic_t running = 1;

void sig_handler(int signum) {
    if (signum == SIGINT) {
        printf("\nSinal de interrupção recebido. Fechando o monitor...\n");
        running = 0;
    }
}

void cleanup() {
    if (raw_socket_fd != -1) {
        close(raw_socket_fd);
    }
    printf("Monitor de Tráfego encerrado.\n");
}

int main(int argc, char *argv[]) {

    if(argc != 2) {
        fprintf(stderr, "Uso: %s <interface>  (ex.: %s tun0)\n", argv[0], argv[0]);
        return 2;
    }
    const char* ifname = argv[1];

    //imprime interface a ser capturada
    printf("Interface %s- \n\n", argv[1]);
    // Configura o tratamento do sinal CTRL+C
    signal(SIGINT, sig_handler);

    // Inicializa os arquivos de log
    initialize_log_files();

    // Cria e configura o Raw Socket
    raw_socket_fd = setup_raw_socket(ifname);
    if (raw_socket_fd < 0) {
        return 1;
    }
    
    printf("Iniciando monitoramento de tráfego na interface tun0. Pressione CTRL+C para parar.\n");
    int packet_count_since_draw = 0;
    // Loop principal de captura
    while (running) {
        // 1. Captura o pacote 
        int size = capture_packet(raw_socket_fd, packet_buffer);
        
        if (size > 0) {
            // 2. Processa e analisa (classifica) o pacote 
            process_packet(packet_buffer, size, &packet_info);
            
            
            update_stats(&packet_info); // atualiza estatísticas
   
            // 3. Escreve nos arquivos de log (e faz a exibição em modo texto, omitida aqui)
            write_network_log(&packet_info);
            write_transport_log(&packet_info);
            write_application_log(&packet_info);
            packet_count_since_draw++;

        }
        //para evitar sobrecarga de recursos, escreve a cada 20 pacotes
        if (packet_count_since_draw >= 20) {
            draw_interface();
            packet_count_since_draw = 0;
        }
    }
    
    cleanup();
    cleanup_stats();
    return 0;
}