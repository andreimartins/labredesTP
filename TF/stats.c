// stats.c
#include "stats.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h> // Para fflush
#include <stdbool.h> // Para bool

// --- CONFIGURAÇÃO DE PREFIXOS ---
#define TUNNEL_PREFIX_V4 "172.31.66."
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
    
    // Inicializa campos e contadores
    strncpy(new_client->client_ip, ip, INET6_ADDRSTRLEN - 1);
    new_client->total_packets = 0;
    new_client->total_bytes = 0;
    
    // Inicialização dos Contadores de Protocolo
    new_client->tcp_count = 0;
    new_client->udp_count = 0;
    new_client->icmp_count = 0;
    new_client->http_count = 0;
    new_client->dhcp_count = 0;
    new_client->dns_count = 0;
    new_client->ntp_count = 0;
    new_client->other_count = 0;

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

/**
 * Atualiza os contadores, incluindo os contadores de protocolos solicitados.
 */
void update_stats(const PacketInfo *info) {
    if (info == NULL) return;
    
    total_packets_received++;
    total_bytes_received += info->total_len;
    
    // --- FILTRO DUAL STACK ---
    bool is_tun_v4 = (strncmp(info->ip_src, TUNNEL_PREFIX_V4, strlen(TUNNEL_PREFIX_V4)) == 0);
    bool is_tun_v6 = (strncmp(info->ip_src, TUNNEL_PREFIX_V6, strlen(TUNNEL_PREFIX_V6)) == 0);

    if (!is_tun_v4 && !is_tun_v6) {
        return; 
    }

    ClientStats *client = get_or_create_client(info->ip_src);
    if (client) {
        client->total_packets++;
        client->total_bytes += info->total_len;

        // 1. Contadores de Camada de Transporte/Rede
        if (strcmp(info->trans_protocol, "TCP") == 0) {
            client->tcp_count++;
        }
        if (strcmp(info->trans_protocol, "UDP") == 0) {
            client->udp_count++;
        }
        if (strcmp(info->net_protocol, "ICMP") == 0 || strcmp(info->net_protocol, "ICMPv6") == 0) {
            client->icmp_count++;
        }
        
        // 2. Contadores de Camada de Aplicação (solicitados)
        if (strcmp(info->app_protocol, "HTTP") == 0) {
            client->http_count++;
        } else if (strcmp(info->app_protocol, "DHCP") == 0) {
            client->dhcp_count++;
        } else if (strcmp(info->app_protocol, "DNS") == 0) {
            client->dns_count++;
        } else if (strcmp(info->app_protocol, "NTP") == 0) {
            client->ntp_count++;
        }
        
        // 3. Outros (Catch-all para non-IP, ou IP com L4 não mapeado)
        // Se for "outro" na camada de rede (geralmente ARP/L2), conta.
        if (strcmp(info->net_protocol, "outro") == 0) {
            client->other_count++;
        } else if (strcmp(info->net_protocol, "IPv4") == 0 || strcmp(info->net_protocol, "IPv6") == 0) {
             // Se for IP, mas não foi classificado como TCP, UDP, ou ICMP/ICMPv6, é "outro" L4/L7.
            if (strcmp(info->trans_protocol, "TCP") != 0 &&
                strcmp(info->trans_protocol, "UDP") != 0 &&
                (strcmp(info->net_protocol, "ICMP") != 0 && strcmp(info->net_protocol, "ICMPv6") != 0)) 
            {
                client->other_count++;
            }
        }

        // 4. Atualiza dados do Destino Remoto (manutenção do requisito de volume de tráfego)
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

/**
 * Desenha a interface modo texto, com todas as colunas de protocolo solicitadas.
 */
void draw_interface() {
    // Limpar tela e mover cursor para o topo
    printf("\033[2J\033[H"); 
    
    printf("\033[1;32m=== Monitor de Tráfego Dual Stack (IPv4/IPv6) ===\033[0m\n");
    printf("Total Pacotes: \033[1m%lu\033[0m | Volume Total: \033[1m%lu bytes\033[0m\n", 
           total_packets_received, total_bytes_received);
    printf("Filtros: [%s*] e [%s*]\n\n", TUNNEL_PREFIX_V4, TUNNEL_PREFIX_V6);
           
            
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("| %-30s | %-12s | %-12s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s | %-8s |\n", 
           "CLIENTE (TUNEL)", "TOTAL PKTS", "TOTAL BYTES", "TCP", "UDP", "ICMP", "HTTP", "DHCP", "DNS", "NTP", "OUTROS");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    ClientStats *client = client_list_head;
    
    if (client == NULL) {
        printf("| Aguardando dados de clientes...                                                                                                                                                        |\n");
    }

    while (client != NULL) {
        // Linha principal do cliente exibindo os contadores de protocolo
        printf("| \033[1;36m%-30s\033[0m | \033[1m%-12lu\033[0m | %-12lu | %-8lu | %-8lu | %-8lu | %-8lu | %-8lu | %-8lu | %-8lu | %-8lu |\n", 
               client->client_ip, 
               client->total_packets, 
               client->total_bytes,
               client->tcp_count,
               client->udp_count,
               client->icmp_count,
               client->http_count,
               client->dhcp_count,
               client->dns_count,
               client->ntp_count,
               client->other_count);

        printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");
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