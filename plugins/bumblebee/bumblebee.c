#include "../game/game.h"

static void set_bumblebee()
{
	player.texture = "🐝";
}

__attribute__ ((constructor)) static void init()
{
	register_input_handler('b', (struct input_handler) {
		.run_if_dead = false,
		.callback = &set_bumblebee,
	});
}
