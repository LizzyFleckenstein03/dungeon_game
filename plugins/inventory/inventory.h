#ifndef _INVENTORY_H_
#define _INVENTORY_H_
#include "../game/game.h"

struct itemstack
{
	struct item *item;
	int count;
	void *meta;
};

struct item
{
	char *name;
	bool stackable;

	bool (*on_use)(struct itemstack *stack);
	void (*on_destroy)(struct itemstack *stack);
	void (*on_create)(struct itemstack *stack);
};

struct inventory
{
	struct list *stacks;
};

void inventory_add(struct inventory *self, struct itemstack stack);
bool inventory_remove(struct inventory *self, struct item *item);

extern struct inventory player_inventory;
#endif
