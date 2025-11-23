// analyze.c
#include "analyze.h"
#include <time.h>
#include <string.h>    // Incluído para memset, strncpy, strcmp
#include <stdio.h>     // Incluído para snprintf, entre outros
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h> // Para ICMPv6
#include <arpa/inet.h>     // Para inet_ntop e inet_ntoa

// Mapeia IDs de protocolo IP para nomes (Camada de Transporte)
// const char* get_transport_protocol_name(int id) {
//     switch (id) {
//         case 1: return "ICMP";
//         case 6: return "TCP";
//         case 17: return "UDP";
//         case 58: return "ICMPv6"; // Adicionado ICMPv6 (ID 58)
//         default: return "outro";
//     }
// }

// Preenche o campo de Data e Hora
void get_timestamp(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 30, "%Y-%m-%d %H:%M:%S", t);
}

/**
 * Função principal de processamento do pacote.
 */
void process_packet(unsigned char *buffer, int size, PacketInfo *info) {
    memset(info, 0, sizeof(PacketInfo));
    info->total_len = size;
    get_timestamp(info->timestamp);
    
    // 1. Analisa a Camada de Rede (IPv4/IPv6)
    analyze_network_layer(buffer, size, info);
    
    // 2. Se for TCP ou UDP, continua para a camada de transporte
    if (info->next_proto_id == 6 || info->next_proto_id == 17) {
        analyze_transport_layer(buffer, size, info);
    }
    
    // 3. Analisa a Camada de Aplicação
    analyze_application_layer(buffer, size, info);
}

/**
 * Analisa a Camada de Rede (IP e ICMP)
 */
void analyze_network_layer(unsigned char *buffer, int size, PacketInfo *info) {
    // A versão do IP está no primeiro byte, nos 4 bits mais significativos
    unsigned int ip_version = (buffer[0] >> 4);
    
    // Zera os campos IP antes de usar
    memset(info->ip_src, 0, INET6_ADDRSTRLEN);
    memset(info->ip_dst, 0, INET6_ADDRSTRLEN);

    if (ip_version == 4) {
        // --- TRATAMENTO IPv4 ---
        struct iphdr *ip_header = (struct iphdr *)buffer;

        strncpy(info->net_protocol, "IPv4", 9);
        
        // Endereços IP (Usando inet_ntoa)
        struct sockaddr_in source, dest;
        source.sin_addr.s_addr = ip_header->saddr;
        dest.sin_addr.s_addr = ip_header->daddr;
        
        strncpy(info->ip_src, inet_ntoa(source.sin_addr), INET6_ADDRSTRLEN - 1);
        strncpy(info->ip_dst, inet_ntoa(dest.sin_addr), INET6_ADDRSTRLEN - 1);
        
        // Protocolo da próxima camada
        info->next_proto_id = ip_header->protocol; 

        // Se for ICMPv4 (ID 1)
        if (info->next_proto_id == 1) { 
            unsigned int ip_header_len = ip_header->ihl * 4;
            struct icmphdr *icmp_header = (struct icmphdr *)(buffer + ip_header_len);
            strncpy(info->net_protocol, "ICMP", 9);
            snprintf(info->icmp_info, 99, "Type: %d, Code: %d", icmp_header->type, icmp_header->code);
        }
        
    } else if (ip_version == 6) {
        // --- TRATAMENTO IPv6 ---
        if (size < sizeof(struct ip6_hdr)) {
            strncpy(info->net_protocol, "outro", 9);
            return;
        }
        struct ip6_hdr *ip6_header = (struct ip6_hdr *)buffer;

        strncpy(info->net_protocol, "IPv6", 9);
        
        // Endereços IP (Usando inet_ntop, que suporta IPv6)
        inet_ntop(AF_INET6, &(ip6_header->ip6_src), info->ip_src, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(ip6_header->ip6_dst), info->ip_dst, INET6_ADDRSTRLEN);
        
        // Protocolo da próxima camada (Next Header)
        info->next_proto_id = ip6_header->ip6_nxt; 
        
        // Se for ICMPv6 (ID 58)
        if (info->next_proto_id == 58) { 
            // O cabeçalho de transporte começa após o cabeçalho base IPv6 (40 bytes)
            unsigned int ip6_header_len = sizeof(struct ip6_hdr); 
            // struct icmp6_hdr *icmp6_header = (struct icmp6_hdr *)(buffer + ip6_header_len); 
            strncpy(info->net_protocol, "ICMPv6", 9);
            snprintf(info->icmp_info, 99, "ICMPv6 packet (ID 58)");
        }
    } else {
        strncpy(info->net_protocol, "outro", 9);
        info->next_proto_id = 0;
    }
}

/**
 * Analisa a Camada de Transporte (TCP/UDP)
 */
void analyze_transport_layer(unsigned char *buffer, int size, PacketInfo *info) {
    unsigned int header_len = 0;
    unsigned int ip_version = (buffer[0] >> 4);
    
    // 1. Determina o tamanho do cabeçalho de rede (offset para o transporte)
    if (ip_version == 4) {
        // IPv4: tamanho variável (IHL * 4)
        struct iphdr *ip_header = (struct iphdr *)buffer;
        header_len = ip_header->ihl * 4;
    } else if (ip_version == 6) {
        // IPv6: tamanho base fixo (40 bytes), ignorando cabeçalhos de extensão por simplicidade
        header_len = sizeof(struct ip6_hdr); 
    } else {
        return; // Não é um pacote IP conhecido
    }
    
    // Verifica se há espaço para o cabeçalho de transporte (mínimo 8 bytes)
    if (size < header_len + 8) {
        return; 
    }
    
    info->port_src = 0;
    info->port_dst = 0;

    // 2. Analisa o cabeçalho de transporte
    if (info->next_proto_id == 6) { // TCP
        strncpy(info->trans_protocol, "TCP", 9);
        // O cabeçalho TCP/UDP começa após o cabeçalho IP
        struct tcphdr *tcp_header = (struct tcphdr *)(buffer + header_len);
        info->port_src = ntohs(tcp_header->source);
        info->port_dst = ntohs(tcp_header->dest);
    } else if (info->next_proto_id == 17) { // UDP
        strncpy(info->trans_protocol, "UDP", 9);
        struct udphdr *udp_header = (struct udphdr *)(buffer + header_len);
        info->port_src = ntohs(udp_header->source);
        info->port_dst = ntohs(udp_header->dest);
    } else {
        strncpy(info->trans_protocol, "outro", 9);
        info->port_src = 0; 
        info->port_dst = 0;
    }
}

/**
 * Analisa a Camada de Aplicação (baseado em portas conhecidas)
 */
void analyze_application_layer(unsigned char *buffer, int size, PacketInfo *info) {
    int port = info->port_src == 0 ? info->port_dst : info->port_src;
    
    // Se for TCP/UDP, verifica as portas conhecidas
    if (strcmp(info->trans_protocol, "TCP") == 0 || strcmp(info->trans_protocol, "UDP") == 0) {
        switch (port) {
            case 80:
            case 443:
                strncpy(info->app_protocol, "HTTP", 9);
                strncpy(info->app_info, "Web Traffic", 255);
                break;
            case 67:
            case 68:
                strncpy(info->app_protocol, "DHCP", 9);
                strncpy(info->app_info, "Dynamic Host Configuration", 255);
                break;
            case 53:
                strncpy(info->app_protocol, "DNS", 9);
                strncpy(info->app_info, "Domain Name Query", 255);
                break;
            case 123:
                strncpy(info->app_protocol, "NTP", 9);
                strncpy(info->app_info, "Network Time Protocol", 255);
                break;
            default:
                strncpy(info->app_protocol, "outro", 9);
                break;
        }
    } else {
         // Para ICMP ou outros protocolos de rede/transporte sem porta
         strncpy(info->app_protocol, "N/A", 9);
    }
}