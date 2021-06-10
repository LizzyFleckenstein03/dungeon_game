#include <stdio.h>
#include "../game/game.h"

static int score = 0;

void add_score(int s)
{
	score += s;
}

int get_score()
{
	return score;
}

static void render_score(struct winsize ws)
{
	(void) ws;

	printf("\e[32m\e[3mScore:\e[23m %d", score);
}

__attribute__ ((constructor)) static void init()
{
	register_render_component(&render_score);
}
