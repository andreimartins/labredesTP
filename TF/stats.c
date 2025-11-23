// stats.c
#include "stats.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h> // Para fflush
#include <stdbool.h> // Para bool

// --- CONFIGURAÇÃO DE PREFIXOS ---
// Prefixo IPv4 da rede do túnel 
#define TUNNEL_PREFIX_V4 "172.31.66."

// Prefixo IPv6 
#define TUNNEL_PREFIX_V6 "fe80:" 

// Variáveis globais
unsigned long total_packets_received = 0;
unsigned long total_bytes_received = 0;
ClientStats *client_list_head = NULL;

// --- Funções Auxiliares (Gerenciamento de Memória) ---

ClientStats* get_or_create_client(const char *ip) {
    ClientStats *current = client_list_head;
    while (current != NULL) {
        if (strcmp(current->client_ip, ip) == 0) {
            return current;
        }
        current = current->next;
    }
    
    ClientStats *new_client = (ClientStats*)malloc(sizeof(ClientStats));
    if (new_client == NULL) {
        perror("Erro fatal: Falha ao alocar memória para cliente");
        exit(1);
    }
    
    // INET6_ADDRSTRLEN garante espaço suficiente para IPv4 e IPv6
    strncpy(new_client->client_ip, ip, INET6_ADDRSTRLEN - 1);
    new_client->total_packets = 0;
    new_client->total_bytes = 0;
    new_client->remote_head = NULL;
    new_client->next = client_list_head;
    client_list_head = new_client;
    
    return new_client;
}

RemoteStats* get_or_create_remote_stats(ClientStats *client, const PacketInfo *info) {
    RemoteStats *current = client->remote_head;
    while (current != NULL) {
        if (strcmp(current->remote_ip, info->ip_dst) == 0 &&
            current->remote_port == info->port_dst &&
            strcmp(current->protocol, info->trans_protocol) == 0) {
            return current;
        }
        current = current->next;
    }
    
    RemoteStats *new_remote = (RemoteStats*)malloc(sizeof(RemoteStats));
    if (new_remote == NULL) {
        perror("Erro fatal: Falha ao alocar memória para estatísticas remotas");
        return NULL;
    }
    
    strncpy(new_remote->remote_ip, info->ip_dst, INET6_ADDRSTRLEN - 1);
    new_remote->remote_port = info->port_dst;
    strncpy(new_remote->protocol, info->trans_protocol, 9);
    new_remote->packet_count = 0;
    new_remote->byte_count = 0;
    new_remote->next = client->remote_head;
    client->remote_head = new_remote;
    
    return new_remote;
}

// --- Funções Principais ---

void init_stats() {
    printf("\033[2J\033[H"); // Limpa tela
    printf("Monitor de Tráfego Iniciado.\n");
    printf("Filtros ativos: IPv4(%s*) e IPv6(%s*)\n", TUNNEL_PREFIX_V4, TUNNEL_PREFIX_V6);
    fflush(stdout);
}

void update_stats(const PacketInfo *info) {
    if (info == NULL) return;
    
    total_packets_received++;
    total_bytes_received += info->total_len;
    
    // --- FILTRO DUAL STACK (IPv4 e IPv6) ---
    bool is_tun_v4 = (strncmp(info->ip_src, TUNNEL_PREFIX_V4, strlen(TUNNEL_PREFIX_V4)) == 0);
    bool is_tun_v6 = (strncmp(info->ip_src, TUNNEL_PREFIX_V6, strlen(TUNNEL_PREFIX_V6)) == 0);

    // Se NÃO for IPv4 do túnel E NÃO for IPv6 do túnel, ignora.
    if (!is_tun_v4 && !is_tun_v6) {
        return; 
    }

    // 1. Atualiza dados do Cliente
    ClientStats *client = get_or_create_client(info->ip_src);
    if (client) {
        client->total_packets++;
        client->total_bytes += info->total_len;

        // 2. Atualiza dados do Destino Remoto
        // Aceita TCP, UDP e também ICMPv6 (que é importante para vizinhança IPv6)
        if (strcmp(info->trans_protocol, "TCP") == 0 || 
            strcmp(info->trans_protocol, "UDP") == 0 ||
            strcmp(info->trans_protocol, "SCTP") == 0) 
        {
            RemoteStats *remote = get_or_create_remote_stats(client, info);
            if (remote) {
                remote->packet_count++;
                remote->byte_count += info->total_len;
            }
        }
    }
}

void draw_interface() {
    // ANSI: Limpar tela e mover cursor para o topo
    printf("\033[2J\033[H"); 
    
    printf("\033[1;32m=== Monitor de Tráfego Dual Stack (IPv4/IPv6) ===\033[0m\n");
    printf("Total Pacotes: \033[1m%lu\033[0m | Volume Total: \033[1m%lu bytes\033[0m\n", 
           total_packets_received, total_bytes_received);
    printf("Filtros: [%s*] e [%s*]\n\n", TUNNEL_PREFIX_V4, TUNNEL_PREFIX_V6);
           
    // Layout Alargado para IPv6 (Coluna Cliente aumentada para 39 chars)
    printf("------------------------------------------------------------------------------------------\n");
    printf("| %-39s | %-25s | %-6s | %-12s |\n", 
           "CLIENTE (TUNEL)", "DESTINO REMOTO", "PROTO", "PKTS/BYTES");
    printf("------------------------------------------------------------------------------------------\n");

    ClientStats *client = client_list_head;
    
    if (client == NULL) {
        printf("| Aguardando dados de clientes...                                                        |\n");
    }

    while (client != NULL) {
        // Linha principal do cliente
        printf("| \033[1;36m%-39s\033[0m | TOTAL GERAL               | %-6s | %-12lu |\n", 
               client->client_ip, "---", client->total_bytes);

        // Lista de destinos
        RemoteStats *remote = client->remote_head;
        int remote_count = 0;

        while(remote != NULL) {
            if (remote_count < 5) {
                // Formatação ajustada para caber na tela
                char remote_dest_str[30];
                snprintf(remote_dest_str, 30, "%s:%d", remote->remote_ip, remote->remote_port);
                
                printf("| %-39s | %-25s | %-6s | %lu/%lu |\n",
                    " ", remote_dest_str, remote->protocol,
                    remote->packet_count, remote->byte_count);
            }
            remote = remote->next;
            remote_count++;
        }

        if (remote_count > 5) {
            printf("| %-39s | ... e mais %d conexões ...                           |\n", 
                   " ", remote_count - 5);
        }

        printf("------------------------------------------------------------------------------------------\n");
        client = client->next;
    }
    
    printf("\nPressione CTRL+C para encerrar.\n");
    fflush(stdout); 
}

void cleanup_stats() {
    ClientStats *current_client = client_list_head;
    while (current_client != NULL) {
        ClientStats *next_client = current_client->next;
        
        RemoteStats *current_remote = current_client->remote_head;
        while (current_remote != NULL) {
            RemoteStats *next_remote = current_remote->next;
            free(current_remote);
            current_remote = next_remote;
        }
        free(current_client);
        current_client = next_client;
    }
    printf("\nMemória liberada.\n");
}

