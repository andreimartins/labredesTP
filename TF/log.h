// log.h
#ifndef LOG_H
#define LOG_H

#include "protocols.h"

void initialize_log_files();
void write_network_log(const PacketInfo *info);
void write_transport_log(const PacketInfo *info);
void write_application_log(const PacketInfo *info);

#endif // LOG_H