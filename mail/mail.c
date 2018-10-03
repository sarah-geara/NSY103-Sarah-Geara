#include "mail.h"
#include "appointment.h"
#include "../shared/list.h"
#include "../shared/tcpconnection.h"
#include "../shared/tcplistener.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>

int start();
void cleanup();
void handle_connection(tcpconnection_t* c);
void handle_disconnection(tcpconnection_t* c);
void handle_message(tcpconnection_t* c, const char* msg, size_t len);
void handle_exit();
appointment_t* find_appointment_by_connection(tcpconnection_t* c);
appointment_t* find_appointment_by_id(unsigned long id);

tcplistener_t* listener;
list_t* appointments;
pthread_mutex_t mutex;
unsigned int active = 0;

int start()
{
    listener = tcplistener_create(MAIL_HOST, MAIL_PORT, handle_connection, handle_disconnection, handle_message);
    if (listener == NULL)
    {
        return -1;
    }

    pthread_mutex_init(&mutex, NULL);
    appointments = list_create(10);

    int err = tcplistener_start(listener);

    return err;
}

void cleanup()
{
    tcplistener_stop(listener);
    tcplistener_free(listener);
    list_free(appointments, 1);
    pthread_mutex_destroy(&mutex);
}

void handle_connection(tcpconnection_t* c)
{
    pthread_mutex_lock(&mutex);

    printf("Client connected\r\n");

    appointment_t* a = (appointment_t*)calloc(1, sizeof(appointment_t));
    a->connection = c;
    list_push(appointments, a);

    pthread_mutex_unlock(&mutex);
}

void handle_disconnection(tcpconnection_t* c)
{
    pthread_mutex_lock(&mutex);

    printf("Client disconnected\r\n");

    appointment_t* a = find_appointment_by_connection(c);
    list_remove(appointments, a);
    free(a);

    pthread_mutex_unlock(&mutex);

    tcpconnection_disconnect(c);
    tcpconnection_free(c);
}

// format: {id,partnerid}
void handle_message(tcpconnection_t* c, const char* msg, size_t len)
{
    pthread_mutex_lock(&mutex);

    puts(msg);

    if (msg[0] == '{' && msg[len - 1] == '}')
    {
        char* sp = strchr(msg, ',');
        appointment_t* a = find_appointment_by_connection(c);
        a->id = strtoul(msg + 1, NULL, 0);
        a->partnerid = strtoul(sp + 1, NULL, 0);

        appointment_t* waiting = find_appointment_by_id(a->partnerid);
        if (waiting != NULL)
        {
            static char buffer[8];
            int len = sprintf(buffer, "#");
            tcpconnection_send(waiting->connection, buffer, len);
            tcpconnection_send(c, buffer, len);
        }
    }

    pthread_mutex_unlock(&mutex);
}

void handle_exit()
{
    active = 0;
    printf("Exiting...\r\n");
}

appointment_t* find_appointment_by_connection(tcpconnection_t* c)
{
    appointment_t* a;
    for (int i = 0; i < appointments->size; ++i)
    {
        a = (appointment_t*)list_get(appointments, i);
        if (a->connection == c)
        {
            return a;
        }
    }
    return NULL;
}

appointment_t* find_appointment_by_id(unsigned long id)
{
    appointment_t* a;
    for (int i = 0; i < appointments->size; ++i)
    {
        a = (appointment_t*)list_get(appointments, i);
        if (a->id == id)
        {
            return a;
        }
    }
    return NULL;
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
