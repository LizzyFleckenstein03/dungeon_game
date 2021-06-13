#include <stdlib.h>
#include <stddef.h>
#include "../game/game.h"
#include "../movement/movement.h"
#include "../inventory/inventory.h"

static bool use_broken_sword(struct itemstack *stack)
{
	(void) stack;

	return true;
}

static struct item broken_sword = {
	.name = "Broken Sword",
	.stackable = false,

	.on_use = &use_broken_sword,
	.on_destroy = NULL,
	.on_create = NULL,
};

static bool use_sword(struct itemstack *stack)
{
	int x, y;
	x = player.x;
	y = player.y;

	dir_to_xy(last_player_move, &x, &y);

	struct entity *entity = entity_collision_map[x][y];

	if (entity) {
		add_health(entity, -1);

		if (rand() % 100 == 0)
			stack->item = &broken_sword;
	}

	return false;
}

static struct item sword = {
	.name = "Sword",
	.stackable = false,

	.on_use = &use_sword,
	.on_destroy = NULL,
	.on_create = NULL,
};

__attribute__((constructor)) static void init()
{
	inventory_add(&player_inventory, (struct itemstack) {
		.item = &sword,
		.count = 1,
		.meta = NULL,
	});
}
