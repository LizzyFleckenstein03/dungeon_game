#include <stdio.h>
#include "recharge.h"
#include "../game/game.h"

static double recharge_full_timer = 0.0;
static double recharge_timer = 0.0;

static const char *recharge_icon;

bool is_charged()
{
	return recharge_timer <= 0;
}

void recharge(double timer, const char *icon)
{
	recharge_full_timer = recharge_timer = timer;
	recharge_icon = icon;
}

static void render_recharge_meter(struct winsize ws)
{
	int y = ws.ws_row - 1;
	int x = ws.ws_col - 14;

	if (recharge_timer <= 0.0)
		return;

	double frac = (recharge_full_timer - recharge_timer) / recharge_full_timer;

	printf("\e[%d;%dH", y, x);

	printf("%s[", recharge_icon);

	struct color color = {
		(1.0 - frac) * 255,
		frac * 255,
		0,
	};

	set_color(color, true);

	char bar[11];
	sprintf(bar, "%9d%%", (int) (frac * 100));

	int bars = frac * 10;

	for (int i = 0; i < 10; i++) {
		if (i == bars)
			set_color(black, true);
		printf("%c", bar[i]);
	}

	set_color(black, true);

	printf("]");
}

static void recharge_globalstep(double dtime)
{
	if (recharge_timer > 0.0)
		recharge_timer -= dtime;
}

__attribute__ ((constructor)) static void init()
{
	register_render_component(&render_recharge_meter);
	register_globalstep((struct globalstep) {
		.run_if_dead = false,
		.callback = &recharge_globalstep,
	});
}
