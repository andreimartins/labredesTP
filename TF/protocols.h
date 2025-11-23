// protocols.h
#ifndef PROTOCOLS_H // <-- Adicione/Verifique esta linha
#define PROTOCOLS_H // <-- Adicione/Verifique esta linha

#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>

// Definir a estrutura do cabeçalho Ethernet (Camada de Enlace)
// Em raw sockets no Linux, muitas vezes o pacote IP é o primeiro a ser visto,
// mas para completude, definimos o Ethernet.
struct eth_header {
    u_char  dmac[ETH_ALEN];    // Destination MAC address
    u_char  smac[ETH_ALEN];    // Source MAC address
    u_short type;              // Protocol type (e.g., ETH_P_IP)
};

// Estrutura para o cabeçalho IP (IPv4) (Camada de Rede)
// Já existe em <netinet/ip.h> como struct iphdr, mas incluída para clareza.
// struct iphdr;

// Estrutura para o cabeçalho TCP (Camada de Transporte)
// Já existe em <netinet/tcp.h> como struct tcphdr.

// Estrutura para o cabeçalho UDP (Camada de Transporte)
// Já existe em <netinet/udp.h> como struct udphdr.

// Estrutura para o cabeçalho ICMP (Camada de Rede)
// Já existe em <netinet/ip_icmp.h> como struct icmphdr.

// Estrutura para o cabeçalho DHCP (Camada de Aplicação)
// É mais complexo; apenas definimos o início (similar ao BOOTP)
struct dhcp_header {
    u_char op;    // Message type (1 = request, 2 = reply)
    u_char htype; // Hardware address type
    u_char hlen;  // Hardware address length
    // ...
};

// Estrutura para agrupar todas as informações extraídas do pacote
typedef struct PacketInfo_sc{
    char timestamp[30];          // Data e hora da captura
    int total_len;               // Tamanho total do pacote em bytes

    // Camada de Rede (camada_internet.csv)
    char net_protocol[10];       // IPv4, IPv6, ICMP, outro
    char ip_src[INET6_ADDRSTRLEN]; // Endereço IP de origem
    char ip_dst[INET6_ADDRSTRLEN]; // Endereço IP de destino
    int next_proto_id;           // ID do protocolo carregado (TCP=6, UDP=17, etc.)
    char icmp_info[100];         // Informações adicionais do ICMP

    // Camada de Transporte (camada_transporte.csv)
    char trans_protocol[10];     // TCP, UDP, outro
    int port_src;                // Porta de origem
    int port_dst;                // Porta de destino

    // Camada de Aplicação (camada_aplicacao.csv)
    char app_protocol[10];       // HTTP, DHCP, DNS, NTP, outro
    char app_info[256];          // Informações do cabeçalho da Aplicação

} PacketInfo;

#endif // PROTOCOLS_H // <-- Adicione/Verifique esta linha