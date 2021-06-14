#include <stdlib.h>
#include "../loot/loot.h"
#include "../game/game.h"

static struct list *loot_list = NULL;

void register_loot(struct loot loot)
{
	loot_list = add_element(loot_list, make_buffer(&loot, sizeof(struct loot)));
}

static void loot_step(struct entity *self, struct entity_step_data stepdata)
{
	if (stepdata.dx == 0 && stepdata.dy == 0) {
		for (struct list *ptr = loot_list; ptr != NULL; ptr = ptr->next) {
			struct loot *loot = ptr->element;
			if (rand() % loot->chance == 0) {
				inventory_add(&player_inventory, (struct itemstack) {
					.item = loot->item,
					.count = loot->min + (loot->max > loot->min ? rand() % (loot->max - loot->min + 1) : 0),
					.meta = NULL,
				});
			}
		}
		self->remove = true;
	}
}

static struct entity loot_entity = {
	.name = "loot",
	.x = 0,
	.y = 0,
	.color = {0},
	.use_color = false,
	.texture = "🎁",
	.remove = false,
	.meta = NULL,
	.health = 1,
	.max_health = 1,
	.collide_with_entities = false,

	.on_step = &loot_step,
	.on_collide = NULL,
	.on_collide_with_entity = NULL,
	.on_spawn = NULL,
	.on_remove = NULL,
	.on_death = NULL,
	.on_damage = NULL,
};

static void spawn_loot(int x, int y, enum mg_context ctx)
{
	spawn(loot_entity, x, y, NULL);
}

__attribute__((constructor)) static void init()
{
	register_air_function((struct generator_function) {
		.corridor_chance = 0,
		.room_chance = 250,
		.callback = &spawn_loot,
	});
}
