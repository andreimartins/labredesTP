// stats.h
#ifndef STATS_H
#define STATS_H

#include "protocols.h"
#include <stdbool.h>
#include <arpa/inet.h> // Para INET6_ADDRSTRLEN

// --- Estruturas de Dados ---

// Estatísticas por Destino Remoto (mantida para logs/requisitos de volume)
typedef struct RemoteStats_s {
    char remote_ip[INET6_ADDRSTRLEN];
    int remote_port;
    char protocol[10];
    
    unsigned long packet_count;
    unsigned long byte_count;
    
    struct RemoteStats_s *next; // Lista ligada
} RemoteStats;

// Estatísticas por Cliente (o IP na rede túnel)
typedef struct ClientStats_s {
    char client_ip[INET6_ADDRSTRLEN];
    
    unsigned long total_packets;
    unsigned long total_bytes;

    // --- Contadores de Protocolo (para exibição) ---
    // Camada de Transporte/Rede
    unsigned long tcp_count;
    unsigned long udp_count;
    unsigned long icmp_count;
    
    // Camada de Aplicação (Protocolos solicitados)
    unsigned long http_count;
    unsigned long dhcp_count;
    unsigned long dns_count;
    unsigned long ntp_count;
    
    // Outros (L3/L4 não classificados ou L2)
    unsigned long other_count;
    // ------------------------------------------------

    // Lista de máquinas remotas acessadas por este cliente
    RemoteStats *remote_head; 
    
    struct ClientStats_s *next; // Lista ligada
} ClientStats;


// --- Funções de Gerenciamento ---

void init_stats();
void update_stats(const PacketInfo *info);
void draw_interface(); 
void cleanup_stats();

// Variáveis globais para contadores
extern unsigned long total_packets_received;
extern unsigned long total_bytes_received;
extern ClientStats *client_list_head;

#endif // STATS_H