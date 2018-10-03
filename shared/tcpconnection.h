#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "list.h"

#include <pthread.h>
#include <netinet/in.h>

#define SOCK_READ_TIMEOUT 10
#define SOCK_BUFFER_SIZE 2000

typedef struct tcpconnection_t tcpconnection_t;

typedef void(*msg_handler_t)(tcpconnection_t*, const char*, size_t);
typedef void(*disconnect_handler_t)(tcpconnection_t*);

struct tcpconnection_t
{
    int sockfd;
    pthread_t readthread;
    pthread_t pingthread;
    pthread_mutex_t mutex;
    msg_handler_t onmessage;
    disconnect_handler_t ondisconnect;
    unsigned long lastsend;
    unsigned int active;
};

tcpconnection_t* tcpconnection_create(int sockfd, msg_handler_t msghandler, disconnect_handler_t disconnect_handler);
tcpconnection_t* tcpconnection_connect(const char* host, int port, msg_handler_t msghandler, disconnect_handler_t disconnecthandler);
void tcpconnection_disconnect(tcpconnection_t* c);
void tcpconnection_free(tcpconnection_t* c);
void tcpconnection_send(tcpconnection_t* c, const char* str, size_t len);

#endif
