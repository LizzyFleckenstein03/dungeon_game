#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../game/game.h"

static int score = 0;
static int needed_score = 5;
static int level = 0;
static char *level_symbol = NULL;
static size_t level_symbol_len = 0;
static double score_timer = 0.0;
static double level_timer = 0.0;

static void level_up()
{
	level += 1;
	needed_score = (level + 1) * 5;

	if (level_symbol)
		free(level_symbol);

	get_roman_numeral(level, &level_symbol, &level_symbol_len);

	level_timer = 2.0;
}

void add_score(int s)
{
	score += s;

	if (score >= needed_score) {
		score -= needed_score;
		level_up();
	} else {
		score_timer = 2.0;
	}
}

int get_score()
{
	return score;
}

int get_level()
{
	return level;
}

static void render_score(struct winsize ws)
{
	int bar_flash = clamp(score_timer * 255, 0, 255);
	set_color((struct color) {bar_flash, 255, bar_flash}, true);

	int level_flash = clamp(level_timer * 255, 0, 255);
	set_color((struct color) {level_flash, 128 + (level_flash / 2), level_flash}, false);

	size_t level_len = level_symbol_len > 0 ? 6 + level_symbol_len + 1 : 0;
	char level_display[level_len];

	if (level_len > 0)
		sprintf(level_display, "Level %s", level_symbol);

	int bar = (double) score / (double) needed_score * ws.ws_col;
	int level_start = (ws.ws_col - level_len) / 2;
	int level_end = level_start + level_len - 1;

	for (int i = 0; i < ws.ws_col; i++) {
		if (i == bar)
			printf("\e[49m");

		if (i >= level_start && i < level_end)
			printf("%c", level_display[i - level_start]);
		else
			printf(" ");
	}

	printf("\n");
}

static void score_globalstep(double dtime)
{
	if (level_timer > 0.0)
		level_timer -= dtime;

	if (score_timer > 0.0)
		score_timer -= dtime * 3.0;
}

__attribute__ ((constructor)) static void init()
{
	register_render_component(&render_score);
	register_globalstep((struct globalstep) {
		.run_if_dead = true,
		.callback = &score_globalstep,
	});
}
