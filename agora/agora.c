#include "agora.h"
#include "../shared/tcpconnection.h"
#include "../shared/tcplistener.h"
#include "../shared/pair.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

tcplistener_t* listener;
pthread_mutex_t mutex;
list_t* connections;
unsigned int active;

int start();
void cleanup();
void handle_exit();
void handle_connection(tcpconnection_t* c);
void handle_disconnection(tcpconnection_t* c);
void handle_message(tcpconnection_t* c, const char* msg, size_t len);
void check();

int start()
{
    pthread_mutex_init(&mutex, NULL);

    listener = tcplistener_create(AGORA_HOST, AGORA_PORT, handle_connection, handle_disconnection, handle_message);

    if (listener == NULL)
    {
        return -1;
    }
    
    connections = list_create(2);

    int err = tcplistener_start(listener);

    return err;
}

void cleanup()
{
    tcplistener_stop(listener);
    tcplistener_free(listener);
    list_free(connections, 1);
    pthread_mutex_destroy(&mutex);
}

void handle_connection(tcpconnection_t* c)
{
    pthread_mutex_lock(&mutex);

    pair_t* pair = pair_create(c, NULL);
    list_push(connections, pair);

    puts("New connection\r\n");

    pthread_mutex_unlock(&mutex);
}

void handle_disconnection(tcpconnection_t* c)
{
    pthread_mutex_lock(&mutex);

    puts("Client disconnected\r\n");

    pair_t* p;
    for (int i = 0; i < connections->size; ++i)
    {
        p = (pair_t*)list_get(connections, i);

        if (p->first == c)
        {
            if (p->second != NULL)
            {
                free(p->second);
                p->second = NULL;
            }

            list_removeat(connections, i);
            pair_free(p);
            
            break;
        }
    }

    pthread_mutex_unlock(&mutex);

    tcpconnection_disconnect(c);
    tcpconnection_free(c);
}

void handle_message(tcpconnection_t* c, const char* msg, size_t len)
{
    pthread_mutex_lock(&mutex);

    if (msg[0] == '{' && msg[len - 1] == '}')
    {
        pair_t* p = NULL;
        for (int i = 0; i < connections->size; ++i)
        {
            p = (pair_t*)list_get(connections, i);

            if (p->first == c)
                break;
        }

        char* ticket = (char*)calloc(len + 1, sizeof(char));
        strcpy(ticket, msg);

        if (p->second != NULL)
            free(p->second);

        p->second = ticket;

        puts("Player queued");
    }

    if (connections->size > 1)
    {
        check();
    }

    pthread_mutex_unlock(&mutex);
}

void handle_exit()
{
    active = 0;
    puts("Exiting...");
}

void check()
{
    pair_t* player1 = NULL;
    pair_t* player2 = NULL;

    pair_t* p = NULL;
    for (int i = 0; i < connections->size; ++i)
    {
        p = (pair_t*)list_get(connections, i);

        if (p->second != NULL)
        {
            if (player1 == NULL)
                player1 = p;
            else
                player2 = p;
        }

        if (player1 != NULL && player2 != NULL)
            break;
    }

    if (player1 != NULL && player2 != NULL)
    {
        const char* ticket1 = (const char*)player1->second;
        const char* ticket2 = (const char*)player2->second;

        tcpconnection_t* c1 = (tcpconnection_t*)player1->first;
        tcpconnection_t* c2 = (tcpconnection_t*)player2->first;

        tcpconnection_send(c1, ticket2, strlen(ticket2));
        tcpconnection_send(c2, ticket1, strlen(ticket1));

        free((char*)player1->second);
        free((char*)player2->second);
        player1->second = NULL;
        player2->second = NULL;
    }
}

/** MAIN **/

int main()
{
    signal(SIGINT, handle_exit);

    int err = start();
    if (err != 0)
    {
        return err;
    }

    active = 1;

    while (active)
    {
        sleep(1);
    }

    cleanup();

    return 0;
}
