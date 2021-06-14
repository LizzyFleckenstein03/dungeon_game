#include <stdlib.h>
#include <stddef.h>
#include "../game/game.h"
#include "../movement/movement.h"
#include "../inventory/inventory.h"
#include "../loot/loot.h"

struct fireball_data
{
	double timer;
	int vx;
	int vy;
};

static void fireball_spawn(struct entity *self, void *data)
{
	self->meta = malloc(sizeof(struct fireball_data));
	*((struct fireball_data *) self->meta) = *((struct fireball_data *) data);

	self->color.r = clamp(self->color.r + rand() % 65 - 32, 0, 255);
	self->color.g = clamp(self->color.g + rand() % 65 - 32, 0, 255);
	self->color.b = clamp(self->color.b + rand() % 65 - 32, 0, 255);
}

static void fireball_step(struct entity *self, struct entity_step_data stepdata)
{
	struct fireball_data *data = self->meta;

	if (stepdata.visible && (data->timer -= stepdata.dtime) <= 0.0) {
		data->timer = 0.1;
		move(self, data->vx, data->vy);
	}
}

static void fireball_collide(struct entity *self, int x, int y)
{
	self->remove = true;
}

static void fireball_collide_with_entity(struct entity *self, struct entity *other)
{
	add_health(other, -(3 + rand() % 3));
	self->remove = true;
}

static struct entity fireball_entity = {
	.name = "fireball",
	.x = 0,
	.y = 0,
	.color = {0},
	.use_color = true,
	.texture = "⬤ ",
	.remove = false,
	.meta = NULL,
	.health = 1,
	.max_health = 1,
	.collide_with_entities = true,

	.on_step = &fireball_step,
	.on_collide = &fireball_collide,
	.on_collide_with_entity = &fireball_collide_with_entity,
	.on_spawn = &fireball_spawn,
	.on_remove = NULL,
	.on_death = NULL,
	.on_damage = NULL,
};

static void shoot_fireball()
{
	int vx, vy;
	vx = vy = 0;

	dir_to_xy(last_player_move, &vx, &vy);

	spawn(fireball_entity, player.x + vx, player.y + vy, & (struct fireball_data) {
		.timer = 0.1,
		.vx = vx,
		.vy = vy,
	});
}

static bool shoot_fireball_item(struct itemstack *stack)
{
	shoot_fireball();
	return true;
}

static struct item fireball_item = {
	.name = "Fireball",
	.stackable = true,

	.on_use = &shoot_fireball_item,
	.on_destroy = NULL,
	.on_create = NULL,
};

static void shoot_if_has_fireball()
{
	if (inventory_remove(&player_inventory, &fireball_item))
		shoot_fireball();
}

__attribute__((constructor)) static void init()
{
	fireball_entity.color = get_color("#FF6611");

	register_input_handler(' ', (struct input_handler) {
		.run_if_dead = false,
		.callback = &shoot_if_has_fireball,
	});

	inventory_add(&player_inventory, (struct itemstack) {
		.item = &fireball_item,
		.count = 7,
		.meta = NULL,
	});

	register_loot((struct loot) {
		.item = &fireball_item,
		.chance = 3,
		.min = 4,
		.max = 7,
	});
}
