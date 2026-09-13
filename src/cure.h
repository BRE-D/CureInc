#ifndef CURE_H
#define CURE_H
#include "types.h"

#define SCIENTIST_COST 100.0f // Scientist-এর দাম; button-ও এই মান পড়ে।
#define LAB_BASE_COST 150.0f // পরের lab upgrade-এর দাম = এটি * (level + 1)।
#define PRODUCTION_BASE_COST 200.0f // পরের production upgrade-এর মূল দাম।

void cure_init(CureState *c); // নতুন খেলার research, টাকা ও vaccine-এর প্রাথমিক মান।
void cure_update(GameState *gs, float dtDays); // প্রতিদিন আয় যোগ করি; ধাপ অনুযায়ী research, উৎপাদন বা বিতরণ করি।

float cure_research_rate(const GameState *gs); // আসল research/day; simulation এবং UI দুটোই এই একই formula ব্যবহার করে।
float cure_production_rate(const CureState *c); // প্রতিদিন কয় unit vaccine তৈরি হবে; scientist ও upgrade উৎপাদন বাড়ায়।
float cure_phase_progress(const CureState *c); // বর্তমান ধাপের সঠিক progress: research, stock অথবা vaccination।

int  cure_hire_scientist(CureState *c); // টাকা যথেষ্ট হলে scientist কিনি; সফল হলে ১, না হলে ০ ফেরত দিই।
int  cure_upgrade_lab(CureState *c); // টাকা ও সর্বোচ্চ level পরীক্ষা করে laboratory উন্নত করি।
int  cure_upgrade_production(CureState *c); // উৎপাদন level বাড়িয়ে নতুন দৈনিক ক্ষমতা সঙ্গে সঙ্গে বসাই।

#endif
