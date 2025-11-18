// gcc -O2 -Wall -Wextra -o monitor monitor.c
// sudo ./monitor enp2s0

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>


static volatile sig_atomic_t g_running = 1;
static void on_sigint(int sig) { (void)sig; g_running = 0; }

static void die(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vfprintf(stderr, fmt, ap); va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

static void now_iso8601(char* buf, size_t n) {
    struct timespec ts; clock_gettime(CLOCK_REALTIME, &ts);
    struct tm tm; localtime_r(&ts.tv_sec, &tm);
    strftime(buf, n, "%Y-%m-%d %H:%M:%S", &tm);
}


static FILE *f_internet=NULL, *f_transporte=NULL, *f_aplicacao=NULL;

static void csv_open_or_die(void) {
    bool new_internet=false, new_transp=false, new_aplic=false;

    f_internet = fopen("internet.csv", "a+");
    f_transporte = fopen("transporte.csv", "a+");
    f_aplicacao = fopen("aplicacao.csv", "a+");
    if(!f_internet||!f_transporte||!f_aplicacao) die("Erro abrindo CSVs: %s", strerror(errno));

    fseek(f_internet, 0, SEEK_END); if(ftell(f_internet)==0) new_internet=true; rewind(f_internet);
    fseek(f_transporte,0, SEEK_END); if(ftell(f_transporte)==0) new_transp=true; rewind(f_transporte);
    fseek(f_aplicacao,0, SEEK_END); if(ftell(f_aplicacao)==0) new_aplic=true; rewind(f_aplicacao);

    if(new_internet) {
        fprintf(f_internet, "timestamp,protocolo,ip_origem,ip_destino,protocolo_carreado,info,tamanho_bytes\n");
        fflush(f_internet);
    }
    if(new_transp) {
        fprintf(f_transporte, "timestamp,protocolo,ip_origem,porta_origem,ip_destino,porta_destino,tamanho_bytes\n");
        fflush(f_transporte);
    }
    if(new_aplic) {
        fprintf(f_aplicacao, "timestamp,protocolo_aplicacao,informacoes\n");
        fflush(f_aplicacao);
    }
}

static void csv_close(void){
    if(f_internet) fclose(f_internet);
    if(f_transporte) fclose(f_transporte);
    if(f_aplicacao) fclose(f_aplicacao);
}


struct Counters {
    unsigned long long ipv4, ipv6, icmp, tcp, udp, other_net;
    unsigned long long app_http, app_dns, app_dhcp, app_ntp, app_other;
    unsigned long long bytes_total;
} g_cnt = {0};


static int find_ip_offset(const unsigned char* buf, int len) {
    int candidates[] = {0, 14, 16, 20};
    for(size_t i=0;i<sizeof(candidates)/sizeof(candidates[0]);++i){
        int off = candidates[i];
        if(len - off < 1) continue;
        unsigned char v = buf[off] >> 4;
        if((v==4 && len - off >= (int)sizeof(struct iphdr)) ||
           (v==6 && len - off >= 40)) {
            return off;
        }
    }
    return -1;
}


static const char* guess_app_name(uint8_t proto, uint16_t sport, uint16_t dport) {
    if(proto == IPPROTO_UDP) {
        if(sport==67 || sport==68 || dport==67 || dport==68) return "DHCP";
        if(sport==53 || dport==53) return "DNS";
        if(sport==123 || dport==123) return "NTP";
    }
    if(proto == IPPROTO_TCP) {
        if(sport==80 || dport==80 || sport==8080 || dport==8080 || sport==8000 || dport==8000) return "HTTP";
    }
    return "outro";
}


static void ui_draw(const char* ifname) {
    printf("\033[2J\033[H");
    printf("Monitor de Tráfego - Interface: %s (RAW)\n", ifname);
    printf("Pressione Ctrl+C para sair.\n\n");

    printf("Camada Internet/ Rede:\n");
    printf("  IPv4: %-12llu  IPv6: %-12llu  ICMP: %-12llu  Outro: %-12llu\n",
           g_cnt.ipv4, g_cnt.ipv6, g_cnt.icmp, g_cnt.other_net);

    printf("\nCamada Transporte:\n");
    printf("  TCP:  %-12llu  UDP:  %-12llu\n", g_cnt.tcp, g_cnt.udp);

    printf("\nCamada Aplicação (heurística por portas):\n");
    printf("  HTTP: %-12llu  DNS: %-12llu  DHCP: %-12llu  NTP: %-12llu  Outro: %-12llu\n",
           g_cnt.app_http, g_cnt.app_dns, g_cnt.app_dhcp, g_cnt.app_ntp, g_cnt.app_other);

    printf("\nBytes totais capturados: %llu\n", g_cnt.bytes_total);
    fflush(stdout);
}


static int open_raw_socket_on(const char* ifname) {
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(fd < 0) die("socket(AF_PACKET) falhou: %s", strerror(errno));

    if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ifname, (socklen_t)strlen(ifname)) < 0) {
        die("SO_BINDTODEVICE(%s) falhou: %s", ifname, strerror(errno));
    }

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    return fd;
}


int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Uso: %s <interface>  (ex.: %s tun0)\n", argv[0], argv[0]);
        return 2;
    }
    const char* ifname = argv[1];

    signal(SIGINT, on_sigint);
    csv_open_or_die();

    int fd = open_raw_socket_on(ifname);

    unsigned char buf[1<<16]; // 64KB
    struct timeval last_ui = {0,0};
    gettimeofday(&last_ui, NULL);

    while(g_running) {
        fd_set rfds;
        struct timeval tv;
        struct sockaddr_ll addr;
        socklen_t alen = sizeof(addr);
        ssize_t n;
        struct timeval now;
        long ms;

        FD_ZERO(&rfds); FD_SET(fd, &rfds);
        tv.tv_sec = 0; tv.tv_usec = 200000; // 200ms

        int rv = select(fd+1, &rfds, NULL, NULL, &tv);
        if(rv < 0 && errno != EINTR) die("select falhou: %s", strerror(errno));

        if(rv > 0 && FD_ISSET(fd, &rfds)) {
            n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &alen);
            if(n < 0) {
                if(errno==EAGAIN || errno==EWOULDBLOCK) goto redraw;
                die("recvfrom falhou: %s", strerror(errno));
            }
            if(n == 0) goto redraw;

            g_cnt.bytes_total += (unsigned long long)n;

            int ipoff = find_ip_offset(buf, (int)n);
            if(ipoff < 0) {
                g_cnt.other_net++;
                goto redraw;
            }

            char ts[32]; now_iso8601(ts, sizeof(ts));

            uint8_t ver = buf[ipoff] >> 4;
            if(ver == 4) {
                if((int)n - ipoff < (int)sizeof(struct iphdr)) { g_cnt.other_net++; goto redraw; }
                struct iphdr* ip = (struct iphdr*)(buf + ipoff);
                g_cnt.ipv4++;

                char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &ip->saddr, src, sizeof(src));
                inet_ntop(AF_INET, &ip->daddr, dst, sizeof(dst));

                uint16_t totlen = ntohs(ip->tot_len);
                uint8_t proto = ip->protocol;

                const char* proto_name = (proto==IPPROTO_ICMP) ? "ICMP" : "IPv4";
                char info[128] = "";
                if(proto==IPPROTO_ICMP && (int)n - ipoff >= (int)(sizeof(struct iphdr)+sizeof(struct icmphdr))) {
                    struct icmphdr* icmp = (struct icmphdr*)((unsigned char*)ip + ip->ihl*4);
                    snprintf(info, sizeof(info), "type=%u code=%u", icmp->type, icmp->code);
                }
                fprintf(f_internet, "%s,%s,%s,%s,%u,%s,%u\n",
                        ts, proto_name, src, dst, (unsigned)proto, info, (unsigned)totlen);
                fflush(f_internet);

                unsigned ihl = ip->ihl * 4u;
                if((int)(ipoff + ihl) >= (int)n) goto redraw;

                if(proto == IPPROTO_TCP) {
                    g_cnt.tcp++;
                    if((int)n - (ipoff + ihl) < (int)sizeof(struct tcphdr)) goto redraw;
                    struct tcphdr* tcp = (struct tcphdr*)(buf + ipoff + ihl);
                    uint16_t sport = ntohs(tcp->source), dport = ntohs(tcp->dest);
                    fprintf(f_transporte, "%s,TCP,%s,%u,%s,%u,%u\n",
                            ts, src, sport, dst, dport, (unsigned)totlen);
                    fflush(f_transporte);

                    const char* app = guess_app_name(IPPROTO_TCP, sport, dport);
                    if(strcmp(app,"HTTP")==0) g_cnt.app_http++;
                    else g_cnt.app_other++;

                    if(strcmp(app,"HTTP")==0) {
                        fprintf(f_aplicacao, "%s,HTTP,portas %u->%u\n", ts, sport, dport);
                    } else {
                        fprintf(f_aplicacao, "%s,outro,proto TCP portas %u->%u\n", ts, sport, dport);
                    }
                    fflush(f_aplicacao);
                } else if(proto == IPPROTO_UDP) {
                    g_cnt.udp++;
                    if((int)n - (ipoff + ihl) < (int)sizeof(struct udphdr)) goto redraw;
                    struct udphdr* udp = (struct udphdr*)(buf + ipoff + ihl);
                    uint16_t sport = ntohs(udp->source), dport = ntohs(udp->dest);
                    fprintf(f_transporte, "%s,UDP,%s,%u,%s,%u,%u\n",
                            ts, src, sport, dst, dport, (unsigned)totlen);
                    fflush(f_transporte);

                    const char* app = guess_app_name(IPPROTO_UDP, sport, dport);
                    if(strcmp(app,"DNS")==0) { g_cnt.app_dns++; }
                    else if(strcmp(app,"DHCP")==0) { g_cnt.app_dhcp++; }
                    else if(strcmp(app,"NTP")==0) { g_cnt.app_ntp++; }
                    else { g_cnt.app_other++; }

                    fprintf(f_aplicacao, "%s,%s,portas %u->%u\n", ts, app, sport, dport);
                    fflush(f_aplicacao);
                } else if(proto == IPPROTO_ICMP) {
                    g_cnt.icmp++;
                } else {
                    g_cnt.other_net++;
                }

            } else if(ver == 6) {
                if((int)n - ipoff < 40) { g_cnt.other_net++; goto redraw; }
                g_cnt.ipv6++;

                const unsigned char* ip6 = buf + ipoff;
                uint8_t next = ip6[6];
                uint16_t payload_len = (ip6[4]<<8) | ip6[5];
                uint16_t totlen = payload_len + 40;

                char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, ip6+8, src, sizeof(src));
                inet_ntop(AF_INET6, ip6+24, dst, sizeof(dst));

                fprintf(f_internet, "%s,IPv6,%s,%s,%u,,%u\n",
                        ts, src, dst, (unsigned)next, (unsigned)totlen);
                fflush(f_internet);

                int l4off = ipoff + 40;
                if(l4off >= (int)n) goto redraw;

                if(next == IPPROTO_TCP) {
                    g_cnt.tcp++;
                    if((int)n - l4off < (int)sizeof(struct tcphdr)) goto redraw;
                    struct tcphdr* tcp = (struct tcphdr*)(buf + l4off);
                    uint16_t sport = ntohs(tcp->source), dport = ntohs(tcp->dest);
                    fprintf(f_transporte, "%s,TCP,%s,%u,%s,%u,%u\n",
                            ts, src, sport, dst, dport, (unsigned)totlen);
                    fflush(f_transporte);

                    const char* app = guess_app_name(IPPROTO_TCP, sport, dport);
                    if(strcmp(app,"HTTP")==0) g_cnt.app_http++;
                    else g_cnt.app_other++;

                    if(strcmp(app,"HTTP")==0) {
                        fprintf(f_aplicacao, "%s,HTTP,portas %u->%u\n", ts, sport, dport);
                    } else {
                        fprintf(f_aplicacao, "%s,outro,proto TCP portas %u->%u\n", ts, sport, dport);
                    }
                    fflush(f_aplicacao);
                } else if(next == IPPROTO_UDP) {
                    g_cnt.udp++;
                    if((int)n - l4off < (int)sizeof(struct udphdr)) goto redraw;
                    struct udphdr* udp = (struct udphdr*)(buf + l4off);
                    uint16_t sport = ntohs(udp->source), dport = ntohs(udp->dest);
                    fprintf(f_transporte, "%s,UDP,%s,%u,%s,%u,%u\n",
                            ts, src, sport, dst, dport, (unsigned)totlen);
                    fflush(f_transporte);

                    const char* app = guess_app_name(IPPROTO_UDP, sport, dport);
                    if(strcmp(app,"DNS")==0) { g_cnt.app_dns++; }
                    else if(strcmp(app,"DHCP")==0) { g_cnt.app_dhcp++; }
                    else if(strcmp(app,"NTP")==0) { g_cnt.app_ntp++; }
                    else { g_cnt.app_other++; }

                    fprintf(f_aplicacao, "%s,%s,portas %u->%u\n", ts, app, sport, dport);
                    fflush(f_aplicacao);
                } else if(next == IPPROTO_ICMPV6) {
                    g_cnt.other_net++;
                } else {
                    g_cnt.other_net++;
                }
            } else {
                g_cnt.other_net++;
            }
        }

redraw:
        gettimeofday(&now, NULL);
        ms = (now.tv_sec - last_ui.tv_sec)*1000L + (now.tv_usec - last_ui.tv_usec)/1000L;
        if(ms >= 200) { ui_draw(ifname); last_ui = now; }
    }

    printf("\nEncerrando...\n");
    close(fd);
    csv_close();
    return 0;
}