#include "cameneon.h"
#include "../shared/tcpconnection.h"
#include "../agora/agora.h"
#include "../mail/mail.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"

struct cameneon_t self;
struct cameneon_t partner;
tcpconnection_t* agora;
tcpconnection_t* mail;
unsigned int active = 0;
unsigned int waiting = 0;
char* outputcolor = KNRM;

int start();
void cleanup();
void handle_exit();
void handle_message(tcpconnection_t* c, const char* msg, size_t len);
void handle_disconnect(tcpconnection_t* c);
void eat();
void practice();
void play();
void change_color(color_t color);

int start()
{
    self.id = getpid();
    change_color(COLOR_MIN + (self.id % COLOR_COUNT));
    return 0;

    // agora = tcpconnection_connect(AGORA_HOST, AGORA_PORT, handle_message, handle_disconnect);
    // mail = tcpconnection_connect(MAIL_HOST, MAIL_PORT, handle_message, handle_disconnect);
}

void cleanup()
{
    if (agora != NULL)
    {
        tcpconnection_disconnect(agora);
    }

    if (mail != NULL)
    {
        tcpconnection_disconnect(mail);
    }
}

void handle_message(tcpconnection_t* c, const char* msg, size_t len)
{
    if (len > 1 && msg[0] == '{' && msg[len - 1] == '}')
    {
        partner.color = strtoul(msg + 1, NULL, 0);
        partner.id = strtoul(msg + 3, NULL, 0);

        waiting = 0;
    }
    else if (len == 1 && msg[0] == '#')
    {
        printf("%sMy partner has arrived, we're playing\r\n", outputcolor);
        sleep(2);

        if (self.color != partner.color)
        {
            color_t newcolor = (COLOR_COUNT - self.color - partner.color) % COLOR_COUNT;
            change_color(newcolor);
            printf("%sChanged color\r\n", outputcolor);
        }
        else
        {
            printf("%sNot changing color\r\n", outputcolor);
        }

        waiting = 0;
    }
}

void handle_disconnect(tcpconnection_t* c)
{
    waiting = 0;
    tcpconnection_free(c);
}

void handle_exit()
{
    active = 0;
    waiting = 0;
    printf("%sExiting...\r\n", outputcolor);
}

void eat()
{
    printf("%sI'm eating\r\n", outputcolor);
    sleep(2);
}

void practice()
{
    printf("%sI'm practicing\r\n", outputcolor);
    sleep(2);
}

void play()
{
    agora = tcpconnection_connect(AGORA_HOST, AGORA_PORT, handle_message, handle_disconnect);
    if (agora == NULL)
    {
        printf("%sAgora is closed\r\n", outputcolor);
        return;
    }


    printf("%sI'm looking for a partner\r\n", outputcolor);
    partner.id = 0;
    waiting = 1;

    sleep(1);

    static char msg[64];
    int len = sprintf(msg, "{%u,%u}", self.color, self.id);
    tcpconnection_send(agora, msg, len);
    while (active && waiting)
    {
        sleep(1);
    }

    if (partner.id != 0)
        printf("%sPartner found: %u\r\n", outputcolor, partner.id);
    
    tcpconnection_disconnect(agora);

    if (!active)
        return;

    printf("%sGoing to mail\r\n", outputcolor);
    mail = tcpconnection_connect(MAIL_HOST, MAIL_PORT, handle_message, handle_disconnect);
    if (mail == NULL)
    {
        printf("%sMail is closed\r\n", outputcolor);
        return;
    }

    printf("%sI'm waiting for my partner at the mail\r\n", outputcolor);
    waiting = 1;

    sleep(1);

    len = sprintf(msg, "{%u,%u}", self.id, partner.id);
    tcpconnection_send(mail, msg, len);
    while (active & waiting)
    {
        sleep(1);
    }
    
    tcpconnection_disconnect(mail);
}

void change_color(color_t color)
{
    self.color = color;
    switch (color)
    {
        case COLOR_RED:
            outputcolor = KRED;
            break;
        case COLOR_BLUE:
            outputcolor = KBLU;
            break;
        case COLOR_YELLOW:
            outputcolor = KYEL;
            break;
        default:
            outputcolor = KNRM;
            break;
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

    printf("%sConnected\r\n", outputcolor);

    active = 1;

    while (active)
    {
        eat();
        
        if (active)
            practice();

        if (active)
            play();
    }

    cleanup();

    return 0;
}
