#include "../game/game.h"

static void move_up()
{
	move(&player, 0, -1);
}

static void move_left()
{
	move(&player, -1, 0);
}

static void move_down()
{
	move(&player, 0, 1);
}

static void move_right()
{
	move(&player, 1, 0);
}

__attribute__((constructor)) static void init()
{
	register_input_handler('w', (struct input_handler) {
		.run_if_dead = false,
		.callback = &move_up,
	});

	register_input_handler('a', (struct input_handler) {
		.run_if_dead = false,
		.callback = &move_left,
	});

	register_input_handler('s', (struct input_handler) {
		.run_if_dead = false,
		.callback = &move_down,
	});

	register_input_handler('d', (struct input_handler) {
		.run_if_dead = false,
		.callback = &move_right,
	});
}
