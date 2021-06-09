#include "../game/game.h"

enum direction last_player_move;

void move_player(enum direction dir)
{
	int x, y;
	x = y = 0;

	dir_to_xy(dir, &x, &y);
	last_player_move = dir;

	move(&player, x, y);
}

static void move_up()
{
	move_player(UP);
}

static void move_left()
{
	move_player(LEFT);
}

static void move_down()
{
	move_player(DOWN);
}

static void move_right()
{
	move_player(RIGHT);
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
