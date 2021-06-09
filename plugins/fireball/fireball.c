#include <stdlib.h>
#include <stddef.h>
#include "../game/game.h"

static struct entity fireball;

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
	(void) x, y;

	self->remove = true;
}

static void fireball_collide_with_entity(struct entity *self, struct entity *other)
{
	add_health(other, -(1 + rand() % 3));
	self->remove = true;
}

static bool try_shoot(int x, int y, int vx, int vy)
{
	x += vx;
	y += vy;

	return spawn(fireball, x, y, & (struct fireball_data) {
		.timer = 0.1,
		.vx = vx,
		.vy = vy,
	});
}

static void shoot_fireball()
{
	int x, y;

	x = player.x;
	y = player.y;

	for (int tries = 10; tries > 0; tries--) {
		int vx, vy;

		vx = vy = 0;

		dir_to_xy(rand() % 4, &vx, &vy);

		if (try_shoot(x, y, vx, vy))
			return;
	}
}

__attribute__((constructor)) static void init()
{
	fireball = (struct entity) {
		.name = "fireball",
		.x = 0,
		.y = 0,
		.color = get_color("#FF6611"),
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

	register_input_handler(' ', (struct input_handler) {
		.run_if_dead = false,
		.callback = &shoot_fireball,
	});
}
