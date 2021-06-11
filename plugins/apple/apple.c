#include <stddef.h>
#include <stdlib.h>
#include "../game/game.h"
#include "../score/score.h"

static void apple_step(struct entity *self, struct entity_step_data stepdata)
{
	if (stepdata.dx == 0 && stepdata.dy == 0) {
		add_score(1);
		add_health(&player, 1);
		self->remove = true;
	}
}

static struct entity apple_entity = {
	.name = "apple",
	.x = 0,
	.y = 0,
	.color = {0},
	.use_color = false,
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
	.on_damage = NULL,
};

static void spawn_apple(int x, int y)
{
	spawn(apple_entity, x, y, NULL);
}

__attribute__((constructor)) static void init()
{
	register_air_function((struct generator_function) {
		.chance = 25,
		.callback = &spawn_apple,
	});
}

