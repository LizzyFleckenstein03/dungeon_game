#include <stddef.h>
#include <stdlib.h>
#include "dungeon.h"

static struct entity apple;

static void apple_step(struct entity *self, struct entity_step_data stepdata)
{
	if (stepdata.dx == 0 && stepdata.dy == 0) {
		add_score(1);
		add_health(&player, 1);
		self->remove = true;
	}
}

static void spawn_apple(int x, int y)
{
	spawn(apple, x, y);
}

__attribute__((constructor)) static void init()
{
	apple = (struct entity) {
		.name = "apple",
		.x = 0,
		.y = 0,
		.color = get_color("#FF2A53"),
		.texture = "🍎",
		.remove = false,
		.meta = NULL,
		.health = 1,
		.max_health = 1,
		.collide_with_entities = false,

		.on_step = &apple_step,
		.on_collide = NULL,
		.on_collide_with_entity = NULL,
		.on_spawn = NULL,
		.on_remove = NULL,
		.on_death = NULL,
	};

	register_air_function((struct generator_function) {
		.chance = 25,
		.callback = &spawn_apple,
	});
}

