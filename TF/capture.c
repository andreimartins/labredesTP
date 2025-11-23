// capture.c
#include "capture.h"
#include <sys/ioctl.h> 
#include <net/if.h> 
#include <linux/if_ether.h> 
#include <string.h>    // Para strlen
#include <stdio.h>     // Para perror, printf
#include <stdlib.h>    // Para exit
#include <errno.h>     // Para strerror(errno)
#include <unistd.h>
// Defina ETH_P_ALL se não estiver em capture.h
// #define ETH_P_ALL 0x0003 


/**
 * Cria um raw socket para captura de pacotes IP.
 * Este socket lerá todos os pacotes que chegam à interface, incluindo cabeçalhos IP.
 * Interface é recebida por parâmetro
 */
int setup_raw_socket(const char* ifname) {

    // AF_PACKET para capturar na Camada de Enlace (inclui cabeçalho Ethernet)
    // ETH_P_ALL para capturar todos os tipos de pacotes na Camada 2
    int sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock_fd < 0) {
        perror("Erro ao criar Raw Socket. Execute com sudo!");
        return -1;
    }
    // int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
    // if (sock_fd < 0) {
    //     perror("Erro ao criar Raw Socket. Execute com sudo!");
    //     return -1;
    // }
    // // 1. Vinculando a interface específica passada por parâmetro (CORRETO)
    if(setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, ifname, (socklen_t)strlen(ifname)) < 0) {
        printf("SO_BINDTODEVICE(%s) falhou: %s", ifname, strerror(errno));
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    
    // 2. Definindo o modo Promíscuo na interface (OPCIONAL, MAS RECOMENDADO)
    struct ifreq ifr;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
    
    if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) == -1) {
        perror("ioctl SIOCGIFFLAGS");
        close(sock_fd);
        return -1;
    }
    
    ifr.ifr_flags |= IFF_PROMISC;
    
    if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) == -1) {
        perror("ioctl SIOCSIFFLAGS - IFF_PROMISC");
        // Não é crítico, pode continuar
    }



    printf("Raw Socket criado com sucesso para captura na interface %s.\n", ifname);
    return sock_fd;
}

/**
 * Captura um pacote do raw socket.
 */
int capture_packet(int sock_fd, unsigned char *buffer) {
    // Recvfrom é usado para obter o tamanho do pacote e, opcionalmente, o endereço de origem
    int data_size = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, NULL, NULL);
    
    if (data_size < 0) {
        // Ignora erros de interrupção (EAGAIN, EINTR)
        if (errno != EINTR && errno != EAGAIN) {
             perror("Erro ao receber o pacote");
        }
    }
    return data_size;
}

