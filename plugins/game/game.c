#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>
#include <pthread.h>
#include "game.h"

bool running = true;
double damage_overlay = 0.0;

int score = 0;

struct color black = {0, 0, 0};

struct material wall;
struct material air;
struct material outside;

struct node map[MAP_WIDTH][MAP_HEIGHT];

struct entity player;
struct list *entities = & (struct list) {
	.element = &player,
	.next = NULL,
};

struct entity *entity_collision_map[MAP_WIDTH][MAP_HEIGHT] = {{NULL}};

struct list *air_functions = NULL;

void quit()
{
	running = false;
}

struct color get_color(const char *str)
{
	unsigned int r, g, b;
	sscanf(str, "#%2x%2x%2x", &r, &g, &b);
	return (struct color) {r, g, b};
}

bool is_outside(int x, int y)
{
	return x >= MAP_WIDTH || x < 0 || y >= MAP_HEIGHT || y < 0;
}

struct node get_node(int x, int y)
{
	return is_outside(x, y) ? (struct node) {&outside} : map[x][y];
}

bool is_solid(int x, int y)
{
	return get_node(x, y).material->solid;
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

void spawn(struct entity def, int x, int y)
{
	if (is_outside(x, y))
		return;

	if (def.collide_with_entities && entity_collision_map[x][y])
		return;

	def.x = x;
	def.y = y;

	struct entity *entity = malloc(sizeof(struct entity));
	*entity = def;

	add_element(entities, entity);

	if (entity->collide_with_entities)
		entity_collision_map[x][y] = entity;

	if (entity->on_spawn)
		entity->on_spawn(entity);
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

void add_score(int s)
{
	score += s;
}

bool player_dead()
{
	return player.health <= 0;
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

void register_air_function(struct generator_function func)
{
	struct generator_function *buf = malloc(sizeof(struct generator_function));
	*buf = func;

	air_functions = add_element(air_functions, buf);
}

/* Player */

static void player_death(struct entity *self)
{
	self->texture = "☠ ";
}

static void player_damage(struct entity *self, int damage)
{
	damage_overlay = (double) damage * 0.5;
}

/* Mapgen */

static bool check_direction(int x, int y, int dir)
{
	if (dir % 2 == 0)
		return is_solid(x, y + 1) && is_solid(x, y - 1) && (is_solid(x + 1, y) || rand() % 3 > 1) && (is_solid(x - 1, y) || rand() % 3 > 1);
	else
		return is_solid(x + 1, y) && is_solid(x - 1, y) && (is_solid(x, y + 1) || rand() % 3 > 1) && (is_solid(x, y - 1) || rand() % 3 > 1);
}

static void generate_corridor(int lx, int ly, int ldir, bool off)
{
	if (is_outside(lx, ly))
		return;

	/*
	if (off && rand() % 100 == 0)
		return;
	*/

	map[lx][ly] = (struct node) {&air};

	for (struct list *ptr = air_functions; ptr != NULL; ptr = ptr->next) {
		struct generator_function *func = ptr->element;

		if (! func->chance || rand() % func->chance == 0) {
			func->callback(lx, ly);
		}
	}

	int x, y, dir;
	int ret = (ldir + 2) % 4;
	int limit = 50;

	do {
		x = lx;
		y = ly;

		if (rand() % 3 > 1)
			dir = ldir;
		else
			dir = rand() % 4;

		switch (dir) {
			case 0:
				x++;
				break;
			case 1:
				y++;
				break;
			case 2:
				x--;
				break;
			case 3:
				y--;
				break;
		}

	} while (dir == ret || (! check_direction(x, y, dir) && --limit));

	if (limit)
		generate_corridor(x, y, dir, off);

	if (rand() % 20 == 0)
		generate_corridor(lx, ly, ldir, true);
}

static void generate_corridor_random(int x, int y)
{
	int dir = rand() % 4;

	generate_corridor(x, y, dir, false);
	generate_corridor(x, y, (dir + 2) % 4, false);
}

/* Rendering */

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

static bool render_color(struct color color, double light, bool bg)
{
	if (light <= 0.0) {
		set_color(black, bg);
		return false;
	} else {
		if (damage_overlay > 0.0)
			mix_color(&color, get_color("#F20000"), damage_overlay * 2.0);

		light_color(&color, light);

		set_color(color, bg);
		return true;
	}
}

static void render(render_entity_list entity_list)
{
	printf("\e[2J\e[0;0H");

	struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);

	int cols = ws.ws_col / 2 - LIGHT * 2;
	int rows = ws.ws_row / 2 - LIGHT;

	int cols_left = ws.ws_col - cols - (LIGHT * 2 + 1) * 2;
	int rows_left = ws.ws_row - rows - (LIGHT * 2 + 1);

	set_color(black, true);

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

			render_color(node.material->color, light, true);

			struct entity *entity = entity_list[x + LIGHT][y + LIGHT];

			if (entity && render_color(entity->color, light, false))
				printf("%s", entity->texture);
			else
				printf("  ");
		}

		set_color(black, true);

		for (int i = 0; i < cols_left; i++)
			printf(" ");
	}

	for (int i = 0; i < rows_left + 1; i++)
		for (int i = 0; i < ws.ws_col; i++)
			printf(" ");

	printf("\e[0;0H\e[39m");

	printf("\e[32m\e[3mScore:\e[23m %d\e[39m", score);

	printf("\e[0;0");

	for (int i = 0; i < rows; i++)
		printf("\n");

	printf("\t\e[1mInventory\e[22m\n\n");
	printf("\t0x\t\e[3mNothing\e[23m\n");

	printf("\e[0;0H");

	for (int i = 0; i < ws.ws_row - 2; i++)
		printf("\n");

	int hearts_cols = ws.ws_col / 2 - player.max_health;

	for (int i = 0; i < hearts_cols; i++)
		printf(" ");

	set_color((struct color) {255, 0, 0}, false);

	for (int i = 0; i < player.max_health; i++) {
		if (i >= player.health)
			set_color(get_color("#5A5A5A"), false);
		printf("\u2665 ");
	}

	printf("\e[39m\n");
}

/* Input */

static void handle_input(char c)
{
	bool dead = player_dead();

	switch (c) {
		case 'q':
			quit();
			break;
		case 'w':
			dead || move(&player, 0, -1);
			break;
		case 'a':
			dead || move(&player, -1, 0);
			break;
		case 's':
			dead || move(&player, 0, 1);
			break;
		case 'd':
			dead || move(&player, 1, 0);
			break;
	}
}

/* Multithreading */

static void handle_interrupt(int signal)
{
	(void) signal;

	running = false;
}

static void *input_thread(void *unused)
{
	(void) unused;

	while (running)
		handle_input(tolower(fgetc(stdin)));

	return NULL;
}

/* Main Game */

__attribute__ ((constructor)) static void init()
{
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

	player = (struct entity) {
		.name = "player",
		.x = MAP_WIDTH / 2,
		.y = MAP_HEIGHT / 2,
		.color = get_color("#00FFFF"),
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

	entity_collision_map[player.x][player.y] = &player;

	for (int x = 0; x < MAP_WIDTH; x++)
		for (int y = 0; y < MAP_HEIGHT; y++)
			map[x][y] = (struct node) {&wall};
}

void game()
{
	srand(time(0));

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
		double dtime = (double) (ts.tv_sec - ts_old.tv_sec) + (double) (ts.tv_nsec - ts_old.tv_nsec) / 1000000000.0;
		ts_old = ts;

		bool dead = player_dead();

		if (! dead && damage_overlay > 0.0)
			damage_overlay -= dtime;

		render_entity_list render_list = {{NULL}};

		for (struct list **ptr = &entities; *ptr != NULL; ) {
			struct entity *entity = (*ptr)->element;

			if (entity->remove) {
				assert(entity != &player);
				struct list *next = (*ptr)->next;

				if (entity->on_remove)
					entity->on_remove(entity);

				if (entity->meta)
					free(entity->meta);

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
				render_list[dx + LIGHT][dy + LIGHT] = entity;

			if (! dead && entity->on_step)
				entity->on_step(entity, (struct entity_step_data) {
					.dtime = dtime,
					.visible = visible,
					.dx = dx,
					.dy = dy,
				});

			ptr = &(*ptr)->next;
		}

		render(render_list);

		// there is no such thing as glfwSwapBuffers, so we just wait 1 / 60 seconds to prevent artifacts
		usleep(1000000 / 60);
	}

	printf("\e[?1049l\e[?25h");
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtio);
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
