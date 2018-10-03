#include "tcpconnection.h"

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>

void readfunction(void* args);
void pingfunction(void* args);

tcpconnection_t* tcpconnection_create(int sockfd, msg_handler_t msghandler, disconnect_handler_t disconnectionhandler)
{
    tcpconnection_t* c = (tcpconnection_t*)calloc(1, sizeof(tcpconnection_t));
    c->sockfd = sockfd;
    c->onmessage = msghandler;
    c->ondisconnect = disconnectionhandler;
    c->lastsend = 0;
    c->active = 1;

    pthread_mutex_init(&c->mutex, NULL);

    // set socket read timeout
    struct timeval tv;
	tv.tv_sec = SOCK_READ_TIMEOUT;
	tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // no delay
    int one = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

    pthread_create(&c->readthread, NULL, (void*)readfunction, (void*)c);
    pthread_create(&c->pingthread, NULL, (void*)pingfunction, (void*)c);

    return c;
}

tcpconnection_t* tcpconnection_connect(const char* host, int port, msg_handler_t msghandler, disconnect_handler_t disconnecthandler)
{
    struct sockaddr_in addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        perror("Connection error\r\n");
        return NULL;
    }

    return tcpconnection_create(sockfd, msghandler, disconnecthandler);
}

void tcpconnection_disconnect(tcpconnection_t* c)
{
    pthread_mutex_lock(&c->mutex);

    if (c->active != 0)
    {
        c->active = 0;
        shutdown(c->sockfd, SHUT_RDWR);

        if (pthread_self() != c->readthread)
            pthread_join(c->readthread, NULL);

        pthread_join(c->pingthread, NULL);

        if (c->ondisconnect != NULL)
            c->ondisconnect(c);
    }

    pthread_mutex_unlock(&c->mutex);
}

void tcpconnection_free(tcpconnection_t* c)
{
    pthread_mutex_destroy(&c->mutex);
    free(c);
}

void tcpconnection_send(tcpconnection_t* c, const char* str, unsigned long len)
{
    pthread_mutex_lock(&c->mutex);

    if (c->active)
    {
        send(c->sockfd, str, len, 0);
        c->lastsend = time(NULL);
    }

    pthread_mutex_unlock(&c->mutex);
}

void readfunction(void* args)
{
    tcpconnection_t* c = (tcpconnection_t*)args;

    int sockfd = c->sockfd;
    msg_handler_t msghandler = c->onmessage;

    int read;
    static char buffer[SOCK_BUFFER_SIZE];

    while (1)
    {
        read = recv(sockfd, buffer, SOCK_BUFFER_SIZE, 0);

        if (read < 1)
            break;

        if (msghandler != NULL && strcmp(buffer, "\0") != 0)
        {
            msghandler(c, buffer, read);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    if (c->active != 0)
        tcpconnection_disconnect(c);
}

void pingfunction(void* args)
{
    long pinginterval = (SOCK_READ_TIMEOUT / 2);

    if (SOCK_READ_TIMEOUT % 2 != 0)
    {
        pinginterval -= 1;
    }

    if (pinginterval < 1)
    {
        return;
    }

    tcpconnection_t* c = (tcpconnection_t*)args;
    
    long now;
    long delta;

    while (c->active)
    {
        for (int i = 0; i < pinginterval; ++i)
        {
            sleep(1);
            if (c->active == 0)
                return;
        }

        pthread_mutex_lock(&c->mutex);

        now = (long)time(NULL);
        delta = now - c->lastsend;

        pthread_mutex_unlock(&c->mutex);

        if (delta >= pinginterval)
        {
            tcpconnection_send(c, "\0", 1);
        }
    }
}
