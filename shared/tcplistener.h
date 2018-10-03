#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include "tcpconnection.h"

#include <pthread.h>
#include <netinet/in.h>

typedef struct tcplistener_t tcplistener_t;
typedef void(*connection_handler_t)(tcpconnection_t*);

struct tcplistener_t
{
    int sockfd;
    pthread_t listenerthread;
    connection_handler_t onconnect;
    disconnect_handler_t ondisconnect;
    msg_handler_t onmessage;
    unsigned int active;
};

tcplistener_t* tcplistener_create(const char* host, int port, connection_handler_t connectionhandler, disconnect_handler_t disconnectionhandler, msg_handler_t msghandler);
int tcplistener_start(tcplistener_t* l);
void tcplistener_stop(tcplistener_t* l);
void tcplistener_free(tcplistener_t* l);

#endif
