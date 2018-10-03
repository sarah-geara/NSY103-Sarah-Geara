#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include "../shared/colors.h"

typedef struct appointment_t appointment_t;

struct appointment_t
{
    void* connection;
    unsigned long id;
    unsigned long partnerid;
};

#endif
