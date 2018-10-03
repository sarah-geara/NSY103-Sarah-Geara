#include "tcplistener.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>

void* listenfunction(void* args);

tcplistener_t* tcplistener_create(const char* host, int port, connection_handler_t connectionhandler, disconnect_handler_t disconnectionhandler, msg_handler_t msghandler)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // allow address:port reuse
    int one = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Bind error\r\n");
        return NULL;
    }

    tcplistener_t* l = (tcplistener_t*)calloc(1, sizeof(tcplistener_t));

    l->sockfd = sockfd;
    l->onconnect = connectionhandler;
    l->ondisconnect = disconnectionhandler;
    l->onmessage = msghandler;
    l->active = 0;

    return l;
}

int tcplistener_start(tcplistener_t* l)
{
    if (l->active == 0)
    {
        int err = listen(l->sockfd, 5);
        if (err < 0)
        {
            perror("Failed to listen on specified address\r\n");
            return err;
        }

        l->active = 1;
        pthread_create(&l->listenerthread, NULL, (void*)listenfunction, l);
    }

    return 0;
}

void tcplistener_stop(tcplistener_t* l)
{
    if (l->active != 0)
    {
        l->active = 0;
        shutdown(l->sockfd, SHUT_RDWR);
        pthread_join(l->listenerthread, NULL);
    }
}

void tcplistener_free(tcplistener_t* l)
{
    free(l);
}

void* listenfunction(void* args)
{
    tcplistener_t* l = (tcplistener_t*)args;

    int serversocket = l->sockfd;

    int clientsocket;
    struct sockaddr_in client;
    socklen_t len = (socklen_t)sizeof(client);

    while (l->active)
    {
        clientsocket = accept(serversocket, &client, &len);
        if (clientsocket > -1 && l->onconnect != NULL)
        {
            tcpconnection_t* c = tcpconnection_create(clientsocket, l->onmessage, l->ondisconnect);
            l->onconnect(c);
        }
    }
}
