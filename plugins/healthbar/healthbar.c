#include <stdio.h>
#include "../game/game.h"

static struct color red = {255, 0, 0};
static struct color gray;

static void render_healthbar(struct winsize ws)
{
	int y = max(ws.ws_row - 2, 0);
	int x = max(ws.ws_col / 2 - player.max_health, 0);

	printf("\e[%u;%uH", y, x);

	set_color(red, false);

	int health = max(player.health, 0);

	for (int i = 0; i < player.max_health; i++) {
		if (i == health)
			set_color(gray, false);
		printf("♥ ");
	}
}

__attribute__ ((constructor)) static void init()
{
	gray = get_color("#5A5A5A");

	register_render_component(&render_healthbar);
}
