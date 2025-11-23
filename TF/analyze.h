// analyze.h
#ifndef ANALYZE_H
#define ANALYZE_H

#include "protocols.h"

void process_packet(unsigned char *buffer, int size, PacketInfo *info);
void analyze_network_layer(unsigned char *buffer, int size, PacketInfo *info);
void analyze_transport_layer(unsigned char *buffer, int size, PacketInfo *info);
void analyze_application_layer(unsigned char *buffer, int size, PacketInfo *info);

#endif // ANALYZE_H