// log.c
#include "log.h"
#include <stdio.h>
#include <string.h>

#define NET_LOG "camada_internet.csv"
#define TRANS_LOG "camada_transporte.csv"
#define APP_LOG "camada_aplicacao.csv"

/**
 * Inicializa os arquivos de log, garantindo que o cabeçalho CSV exista.
 */
void initialize_log_files() {
    FILE *fp;

    // Camada de Internet (Rede)
    fp = fopen(NET_LOG, "w");
    if (fp) {
        fprintf(fp, "DataHora,Protocolo,IP_Origem,IP_Destino,Protocolo_ID,Outras_Infos,Tamanho_Total\n"); 
        fclose(fp);
    }
    
    // Camada de Transporte
    fp = fopen(TRANS_LOG, "w");
    if (fp) {
        fprintf(fp, "DataHora,Protocolo,IP_Origem,Porta_Origem,IP_Destino,Porta_Destino,Tamanho_Total\n");
        fclose(fp);
    }

    // Camada de Aplicação
    fp = fopen(APP_LOG, "w");
    if (fp) {
        fprintf(fp, "DataHora,Protocolo,Informacoes_Protocolo\n");
        fclose(fp);
    }
}

/**
 * Escreve o log da Camada de Rede (camada_internet.csv).
 */
void write_network_log(const PacketInfo *info) {
    // Abre em modo 'append' (a) para atualização em tempo real 
    FILE *fp = fopen(NET_LOG, "a");
    if (fp) {
        fprintf(fp, "%s,%s,%s,%s,%d,\"%s\",%d\n",
            info->timestamp,
            info->net_protocol,
            info->ip_src,
            info->ip_dst,
            info->next_proto_id,
            strcmp(info->net_protocol, "ICMP") == 0 ? info->icmp_info : "N/A",
            info->total_len
        );
        fclose(fp);
    }
}

/**
 * Escreve o log da Camada de Transporte (camada_transporte.csv).
 */
void write_transport_log(const PacketInfo *info) {
    if (strcmp(info->trans_protocol, "TCP") != 0 && strcmp(info->trans_protocol, "UDP") != 0) {
        return; // Não é protocolo de transporte pedido
    }

    FILE *fp = fopen(TRANS_LOG, "a");
    if (fp) {
        fprintf(fp, "%s,%s,%s,%d,%s,%d,%d\n",
            info->timestamp,
            info->trans_protocol,
            info->ip_src,
            info->port_src,
            info->ip_dst,
            info->port_dst,
            info->total_len
        );
        fclose(fp);
    }
}

/**
 * Escreve o log da Camada de Aplicação (camada_aplicacao.csv).
 */
void write_application_log(const PacketInfo *info) {
    if (strcmp(info->app_protocol, "outro") == 0 || strcmp(info->app_protocol, "N/A") == 0) {
        return; // Não é protocolo de aplicação pedido
    }

    FILE *fp = fopen(APP_LOG, "a");
    if (fp) {
        fprintf(fp, "%s,%s,\"%s\"\n",
            info->timestamp,
            info->app_protocol,
            info->app_info
        );
        fclose(fp);
    }
}