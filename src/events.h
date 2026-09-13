#ifndef EVENTS_H
#define EVENTS_H

#include "types.h"

void events_init(GameState *gs); // নতুন খেলার জন্য পুরোনো খবর ও আগের event মুছে দিই।

void events_trigger_random(GameState *gs); // একটি random event বেছে তার নির্দিষ্ট প্রভাব প্রয়োগ করি।

void events_update(GameState *gs, float delta); // খবরের সময় কমাই; সময় শেষ হলে খবর লুকাই।

void events_add(GameState *gs, const char *title, const char *description); // খালি জায়গায় খবর রাখি; সব ভরা হলে সবচেয়ে আগে শেষ হবে এমন খবরটি সরাই।

#endif
