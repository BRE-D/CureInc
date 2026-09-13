#ifndef VIRUS_H
#define VIRUS_H

#include "types.h"

void virus_init(Virus *v); // নতুন খেলার virus-এর প্রাথমিক মান বসাই।
void virus_update(GameState *gs); // দিনে একবার নতুন সংক্রমণ, মৃত্যু ও সুস্থ হওয়া হিসাব করি।
void virus_refresh_totals(GameState *gs); // সব অঞ্চলের মানুষ গুনে বিশ্বব্যাপী হার বের করি; শতাংশের সরল গড় নয়।

int virus_try_mutate(Virus *v, int day); // আগের mutation-এর ২০–৩০ দিন পরে virus বদলায়।
int virus_has_trait(const Virus *v, MutationTrait t); // & দিয়ে দেখি নির্দিষ্ট বৈশিষ্ট্যের bit চালু আছে কি না।

const char *virus_trait_name(MutationTrait t); // বৈশিষ্ট্যের code থেকে পর্দায় দেখানোর নাম পাই।
float virus_hospital_load(const Region *r); // রোগীর চাহিদা / শয্যা; ১-এর বেশি হলে হাসপাতাল overloaded।

#endif
