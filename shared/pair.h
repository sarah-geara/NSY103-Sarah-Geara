#ifndef PAIR_H
#define PAIR_H

#include <stdlib.h>

typedef struct pair_t pair_t;

struct pair_t
{
    void* first;
    void* second;
};

inline static pair_t* pair_create(void* first, void* second)
{
    pair_t* pair = (pair_t*)calloc(1, sizeof(pair_t));
    pair->first = first;
    pair->second = second;
    return pair;
}

inline static void pair_free(pair_t* p)
{
    free(p);
}

#endif
