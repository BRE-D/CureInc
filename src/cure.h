#ifndef CURE_H
#define CURE_H
#include "types.h"

void cure_init(CureState *c);
void cure_update(CureState *c, float dtDays);

#endif