#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <signal.h>
#include <termios.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include "game.h"

/* Shared variables */

struct color black = {0};

struct material wall;
struct material air;
struct material outside;

struct node map[MAP_WIDTH][MAP_HEIGHT];

struct list *entities = & (struct list) {
	.element = &player,
	.next = NULL,
};

/* Private variables */

struct entity *entity_collision_map[MAP_WIDTH][MAP_HEIGHT] = {{NULL}};

static bool running = true;
static double damage_overlay = 0.0;
static struct color damage_overlay_color;

static struct list *air_functions = NULL;
static struct input_handler *input_handlers[256] = {NULL};
static struct entity *render_entities[LIGHT * 2 + 1][LIGHT * 2 + 1];
static struct list *render_components = NULL;
static struct list *globalsteps = NULL;

/* Helper functions */

struct color get_color(const char *str)
{
	unsigned int r, g, b;
	sscanf(str, "#%2x%2x%2x", &r, &g, &b);
	return (struct color) {r, g, b};
}

void set_color(struct color color, bool bg)
{
	printf("\e[%u;2;%u;%u;%um", bg ? 48 : 38, color.r, color.g, color.b);
}

void light_color(struct color *color, double light)
{
	color->r *= light;
	color->g *= light;
	color->b *= light;
}

void mix_color(struct color *color, struct color other, double ratio)
{
	double ratio_total = ratio + 1;

	color->r = (color->r + other.r * ratio) / ratio_total;
	color->g = (color->g + other.g * ratio) / ratio_total;
	color->b = (color->b + other.b * ratio) / ratio_total;
}

void dir_to_xy(enum direction dir, int *x, int *y)
{
	switch (dir) {
		case UP:
			(*y)--;
			break;
		case LEFT:
			(*x)--;
			break;
		case DOWN:
			(*y)++;
			break;
		case RIGHT:
			(*x)++;
			break;
	}
}

struct list *add_element(struct list *list, void *element)
{
	struct list **ptr;

	for (ptr = &list; *ptr != NULL; ptr = &(*ptr)->next)
		;

	*ptr = malloc(sizeof(struct list));
	(*ptr)->element = element;
	(*ptr)->next = NULL;

	return list;
}

int clamp(int v, int min, int max)
{
	return v < min ? min : v > max ? max : v;
}

int max(int a, int b)
{
	return a > b ? a : b;
}

int min(int a, int b)
{
	return a < b ? a : b;
}

void *make_buffer(void *ptr, size_t size)
{
	void *buf = malloc(size);
	memcpy(buf, ptr, size);

	return buf;
}

double calculate_dtime(struct timespec from, struct timespec to)
{
	return (double) (to.tv_sec - from.tv_sec) + (double) (to.tv_nsec - from.tv_nsec) / 1000000000.0;
}

/*struct roman_conversion_rule
{
	int number;
	const char *symbol;
};*/

static struct
{
	int number;
	const char *symbol;
} roman_ct[13] = {
	{1000, "M"},
	{900, "CM"},
	{500, "D"},
	{400, "CD"},
	{100, "C"},
	{90, "XC"},
	{50, "L"},
	{40, "XL"},
	{10, "X"},
	{9, "IX"},
	{5, "V"},
	{4, "IV"},
	{1, "I"}
};

void get_roman_numeral(int number, char **ptr, size_t *len)
{
	*ptr = NULL;
	*len = 0;

	for (int i = 0; i < 13; i++) {
		while (number >= roman_ct[i].number) {
			number -= roman_ct[i].number;
			size_t old_len = *len;
			*len += 1 + i % 2;
			*ptr = realloc(*ptr, *len + 1);
			strcpy(*ptr + old_len, roman_ct[i].symbol);
		}
	}
}

/* Game-related utility functions */

void quit()
{
	running = false;
}

bool player_dead()
{
	return player.health <= 0;
}

/* Map functions */

struct node get_node(int x, int y)
{
	return is_outside(x, y) ? (struct node) {&outside} : map[x][y];
}

bool is_outside(int x, int y)
{
	return x >= MAP_WIDTH || x < 0 || y >= MAP_HEIGHT || y < 0;
}

bool is_solid(int x, int y)
{
	return get_node(x, y).material->solid;
}

/* Entity functions */

bool spawn(struct entity def, int x, int y, void *data)
{
	if (is_solid(x, y))
		return false;

	if (def.collide_with_entities && entity_collision_map[x][y])
		return false;

	def.x = x;
	def.y = y;

	struct entity *entity = malloc(sizeof(struct entity));
	*entity = def;

	add_element(entities, entity);

	if (entity->collide_with_entities)
		entity_collision_map[x][y] = entity;

	if (entity->on_spawn)
		entity->on_spawn(entity, data);

	return true;
}

bool move(struct entity *entity, int xoff, int yoff)
{
	int x, y;

	x = entity->x + xoff;
	y = entity->y + yoff;

	if (is_solid(x, y)) {
		if (entity->on_collide)
			entity->on_collide(entity, x, y);
		return false;
	} else if (entity->collide_with_entities && entity_collision_map[x][y]) {
		if (entity->on_collide_with_entity)
			entity->on_collide_with_entity(entity, entity_collision_map[x][y]);
		return false;
	} else {
		entity_collision_map[entity->x][entity->y] = NULL;
		entity->x = x;
		entity->y = y;
		entity_collision_map[entity->x][entity->y] = entity;
		return true;
	}
}

void add_health(struct entity *entity, int health)
{
	bool was_alive = entity->health > 0;

	entity->health += health;

	if (health < 0 && entity->on_damage)
		entity->on_damage(entity, -health);

	if (entity->health > entity->max_health)
		entity->health = entity->max_health;
	else if (entity->health <= 0 && was_alive && entity->on_death)
		entity->on_death(entity);
}

/* Register callback functions */

void register_air_function(struct generator_function func)
{
	air_functions = add_element(air_functions, make_buffer(&func, sizeof(struct generator_function)));
}

void register_input_handler(unsigned char c, struct input_handler handler)
{
	if (input_handlers[c])
		return;

	input_handlers[c] = make_buffer(&handler, sizeof(struct input_handler));
}

void register_render_component(void (*callback)(struct winsize ws))
{
	render_components = add_element(render_components, callback);
};

void register_globalstep(struct globalstep step)
{
	globalsteps = add_element(globalsteps, make_buffer(&step, sizeof(struct globalstep)));
}

/* Player */

static void player_death(struct entity *self)
{
	self->texture = "☠ ";
}

static void player_damage(struct entity *self, int damage)
{
	damage_overlay += (double) damage * 0.5;
}

struct entity player = {
	.name = "player",
	.x = MAP_WIDTH / 2,
	.y = MAP_HEIGHT / 2,
	.color = {0},
	.use_color = false,
	.texture = "🙂",
	.remove = false,
	.meta = NULL,
	.health = 10,
	.max_health = 10,
	.collide_with_entities = true,

	.on_step = NULL,
	.on_collide = NULL,
	.on_collide_with_entity = NULL,
	.on_spawn = NULL,
	.on_remove = NULL,
	.on_death = &player_death,
	.on_damage = &player_damage,
};

/* Mapgen */

static void mapgen_set_air(int x, int y)
{
	if (is_outside(x, y))
		return;

	if (map[x][y].material == &air)
		return;

	map[x][y] = (struct node) {&air};

	for (struct list *ptr = air_functions; ptr != NULL; ptr = ptr->next) {
		struct generator_function *func = ptr->element;

		if (rand() % func->chance == 0)
			func->callback(x, y);
	}
}

static void generate_room(int origin_x, int origin_y)
{
	int left = 5 + rand() % 10;
	int right = 5 + rand() % 10;

	int up = 0;
	int down = 0;

	for (int x = -left; x <= right; x++) {
		if (x < 0) {
			up += rand() % 2;
			down += rand() % 2;
		} else {
			up -= rand() % 2;
			down -= rand() % 2;
		}

		for (int y = -up; y <= down; y++)
			mapgen_set_air(origin_x + x, origin_y + y);
	}
}

static bool check_direction(int x, int y, enum direction dir)
{
	if (dir % 2 == 0)
		return is_solid(x + 1, y) && is_solid(x - 1, y) && (is_solid(x, y + 1) || rand() % 3 > 1) && (is_solid(x, y - 1) || rand() % 3 > 1);
	else
		return is_solid(x, y + 1) && is_solid(x, y - 1) && (is_solid(x + 1, y) || rand() % 3 > 1) && (is_solid(x - 1, y) || rand() % 3 > 1);
}

static void generate_corridor(int lx, int ly, enum direction ldir)
{
	if (is_outside(lx, ly))
		return;

	if (rand() % 200 == 0) {
		generate_room(lx, ly);
		return;
	}

	mapgen_set_air(lx, ly);

	int x, y;

	enum direction dir;
	enum direction ret = (ldir + 2) % 4;

	int limit = 50;

	do {
		x = lx;
		y = ly;

		if (rand() % 3 > 1)
			dir = ldir;
		else
			dir = rand() % 4;

		dir_to_xy(dir, &x, &y);
	} while (dir == ret || (! check_direction(x, y, dir) && --limit));

	if (limit)
		generate_corridor(x, y, dir);

	if (rand() % 20 == 0)
		generate_corridor(lx, ly, ldir);
}

static void generate_corridor_random(int x, int y)
{
	enum direction dir = rand() % 4;

	generate_corridor(x, y, dir);
	generate_corridor(x, y, (dir + 2) % 4);
}

/* Rendering */

static bool render_color(struct color color, double light, bool bg, bool use_color)
{
	if (light <= 0.0) {
		set_color(black, bg);
		return false;
	} else if (use_color) {
		if (damage_overlay > 0.0)
			mix_color(&color, damage_overlay_color, damage_overlay * 2.0);

		light_color(&color, light);

		set_color(color, bg);
		return true;
	} else {
		return true;
	}
}

static void render_map(struct winsize ws)
{
	int cols = ws.ws_col / 2 - LIGHT * 2;
	int rows = ws.ws_row / 2 - LIGHT;

	int cols_left = ws.ws_col - cols - (LIGHT * 2 + 1) * 2;
	int rows_left = ws.ws_row - rows - (LIGHT * 2 + 1);

	for (int i = 0; i < rows; i++)
		for (int i = 0; i < ws.ws_col; i++)
			printf(" ");

	for (int y = -LIGHT; y <= LIGHT; y++) {
		for (int i = 0; i < cols; i++)
			printf(" ");

		for (int x = -LIGHT; x <= LIGHT; x++) {
			int map_x, map_y;

			map_x = x + player.x;
			map_y = y + player.y;

			struct node node = get_node(map_x, map_y);

			double dist = sqrt(x * x + y * y);
			double light = 1.0 - (double) dist / (double) LIGHT;

			render_color(node.material->color, light, true, true);

			struct entity *entity = render_entities[x + LIGHT][y + LIGHT];
			render_entities[x + LIGHT][y + LIGHT] = NULL;

			if (entity && render_color(entity->color, light, false, entity->use_color))
				printf("%s", entity->texture);
			else
				printf("  ");
		}

		set_color(black, true);

		for (int i = 0; i < cols_left; i++)
			printf(" ");
	}

	for (int i = 0; i < rows_left; i++)
		for (int i = 0; i < ws.ws_col; i++)
			printf(" ");
}

static void render()
{
	printf("\e[2J");

	struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

	for (struct list *ptr = render_components; ptr != NULL; ptr = ptr->next) {
		printf("\e[0m\e[0;0H");
		set_color(black, true);

		((void (*)(struct winsize ws)) ptr->element)(ws);
	}

	fflush(stdout);
}

/* Input */

static void handle_interrupt(int signal)
{
	(void) signal;

	quit();
}

static void handle_input(unsigned char c)
{
	struct input_handler *handler = input_handlers[c];

	if (handler && (handler->run_if_dead || ! player_dead()))
		handler->callback();
}

static void *input_thread(void *unused)
{
	(void) unused;

	while (running)
		handle_input(tolower(fgetc(stdin)));

	return NULL;
}

/* Main Game */

void game()
{
	struct sigaction sa;
	sa.sa_handler = &handle_interrupt;
	sigaction(SIGINT, &sa, NULL);

	generate_corridor_random(player.x, player.y);

	for (int i = 0; i < 50; i++)
		generate_corridor_random(rand() % MAP_WIDTH, rand() % MAP_HEIGHT);

	struct termios oldtio, newtio;
	tcgetattr(STDIN_FILENO, &oldtio);
	newtio = oldtio;
	newtio.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newtio);

	printf("\e[?1049h\e[?25l");

	pthread_t input_thread_id;
	pthread_create(&input_thread_id, NULL, &input_thread, NULL);

	struct timespec ts, ts_old;
	clock_gettime(CLOCK_REALTIME, &ts_old);

	while (running) {
		clock_gettime(CLOCK_REALTIME, &ts);
		double dtime = calculate_dtime(ts_old, ts);
		ts_old = ts;

		bool dead = player_dead();

		for (struct list *ptr = globalsteps; ptr != NULL; ptr = ptr->next) {
			struct globalstep *step = ptr->element;

			if (step->run_if_dead || ! dead)
				step->callback(dtime);
		}

		if (! dead && damage_overlay > 0.0) {
			damage_overlay -= dtime;

			if (damage_overlay < 0.0)
				damage_overlay = 0.0;
		}

		for (struct list **ptr = &entities; *ptr != NULL; ) {
			struct entity *entity = (*ptr)->element;

			if (entity->remove) {
				assert(entity != &player);
				struct list *next = (*ptr)->next;

				if (entity->on_remove)
					entity->on_remove(entity);

				if (entity->meta)
					free(entity->meta);

				if (entity->collide_with_entities)
					entity_collision_map[entity->x][entity->y] = NULL;

				free(entity);
				free(*ptr);

				*ptr = next;
				continue;
			}

			int dx, dy;

			dx = entity->x - player.x;
			dy = entity->y - player.y;

			bool visible = abs(dx) <= LIGHT && abs(dy) <= LIGHT;

			if (visible)
				render_entities[dx + LIGHT][dy + LIGHT] = entity;

			if (! dead && entity->on_step)
				entity->on_step(entity, (struct entity_step_data) {
					.dtime = dtime,
					.visible = visible,
					.dx = dx,
					.dy = dy,
				});

			ptr = &(*ptr)->next;
		}

		render();

		// there is no such thing as glfwSwapBuffers, so we just wait 1 / 60 seconds to prevent artifacts
		usleep(1000000 / 60);
	}

	printf("\e[?1049l\e[?25h");
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
}

/* Initializer function */

__attribute__ ((constructor)) static void init()
{
	srand(time(0));

	wall = (struct material) {
		.solid = true,
		.color = get_color("#5B2F00"),
	};

	air = (struct material) {
		.solid = false,
		.color = get_color("#FFE027"),
	};

	outside = (struct material) {
		.solid = true,
		.color = black,
	};

	entity_collision_map[player.x][player.y] = &player;

	for (int x = 0; x < MAP_WIDTH; x++)
		for (int y = 0; y < MAP_HEIGHT; y++)
			map[x][y] = (struct node) {&wall};

	register_input_handler('q', (struct input_handler) {
		.run_if_dead = true,
		.callback = &quit,
	});

	register_render_component(&render_map);

	damage_overlay_color = get_color("#F20000");
}

/* Use later */

/*
get_box_char(is_solid(x, y - 1), is_solid(x, y + 1), is_solid(x - 1, y), is_solid(x + 1, y));

const char *get_box_char(bool up, bool down, bool left, bool right)
{
	if (left && right && ! up && ! down)
		return "\u2501\u2501";
	else if (up && down && ! right && ! left)
		return "\u2503 ";
	else if (down && right && ! up && ! left)
		return "\u250F\u2501";
	else if (down && left && ! up && ! right)
		return "\u2513 ";
	else if (up && right && ! down && ! left)
		return "\u2517\u2501";
	else if (up && left && ! down && ! right)
		return "\u251B ";
	else if (up && down && right && ! left)
		return "\u2523\u2501";
	else if (up && down && left && ! right)
		return "\u252B ";
	else if (down && left && right && ! up)
		return "\u2533\u2501";
	else if (up && left && right && ! down)
		return "\u253B\u2501";
	else if (up && down && left && right)
		return "\u254b\u2501";
	else if (left && ! up && ! down && ! right)
		return "\u2578 ";
	else if (up && ! down && ! left && ! right)
		return "\u2579 ";
	else if (right && ! up && ! down && ! left)
		return "\u257A\u2501";
	else if (down && ! up && ! left && ! right)
		return "\u257B ";
	else if (! up && ! down && ! left && ! right)
		return "\u25AA ";
	else
		return "??";
}
*/
