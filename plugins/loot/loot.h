#ifndef _LOOT_H_
#define _LOOT_H_

#include "../inventory/inventory.h"

struct loot
{
	struct item *item;
	int chance;
	int min;
	int max;
};

void register_loot(struct loot loot);

#endif
