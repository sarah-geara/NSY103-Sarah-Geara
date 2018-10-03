#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H

#define COLOR_RED (color_t)0
#define COLOR_BLUE (color_t)1
#define COLOR_YELLOW (color_t)2

#define COLOR_MIN COLOR_RED
#define COLOR_MAX COLOR_YELLOW
#define COLOR_COUNT (1 + COLOR_MAX - COLOR_MIN)

typedef unsigned int color_t;

#endif
