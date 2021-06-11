#ifndef _GAME_H_
#define _GAME_H_

#include <stdbool.h>
#include <sys/ioctl.h>
#include <stddef.h>
#define MAP_HEIGHT 1000
#define MAP_WIDTH 1000
#define LIGHT 10

/* Type definitions */

struct color
{
	unsigned char r, g, b;
};

struct material
{
	bool solid;
	struct color color;
};

struct node
{
	struct material *material;
};

struct entity_step_data
{
	double dtime;
	int dx;
	int dy;
	bool visible;
};

struct entity
{
	char *name;
	int x, y;
	struct color color;
	bool use_color;
	char *texture;
	bool remove;
	void *meta;
	int health;
	int max_health;
	bool collide_with_entities;

	void (*on_step)(struct entity *self, struct entity_step_data stepdata);
	void (*on_collide)(struct entity *self, int x, int y);
	void (*on_collide_with_entity)(struct entity *self, struct entity *other);
	void (*on_spawn)(struct entity *self, void *data);
	void (*on_remove)(struct entity *self);
	void (*on_death)(struct entity *self);
	void (*on_damage)(struct entity *self, int damage);
};

struct list
{
	void *element;
	struct list *next;
};

struct generator_function
{
	int chance;
	void (*callback)(int x, int y);
};

struct input_handler
{
	bool run_if_dead;
	void (*callback)();
};

enum direction
{
	UP,
	LEFT,
	DOWN,
	RIGHT,
};

extern struct color black;

extern struct material wall;
extern struct material air;
extern struct material outside;

extern struct node map[MAP_WIDTH][MAP_HEIGHT];

extern struct entity player;
extern struct list *entities;

extern struct entity *entity_collision_map[MAP_WIDTH][MAP_HEIGHT];

struct color get_color(const char *str);
void set_color(struct color color, bool bg);
void light_color(struct color *color, double light);
void mix_color(struct color *color, struct color other, double ratio);
void dir_to_xy(enum direction dir, int *x, int *y);
struct list *add_element(struct list *list, void *element);
int clamp(int v, int max, int min);
int max(int a, int b);
int min(int a, int b);
void *make_buffer(void *ptr, size_t size);

void quit();
bool player_dead();

struct node get_node(int x, int y);
bool is_outside(int x, int y);
bool is_solid(int x, int y);

bool spawn(struct entity def, int x, int y, void *data);
bool move(struct entity *entity, int xoff, int yoff);
void add_health(struct entity *entity, int health);

void register_air_function(struct generator_function func);
void register_input_handler(unsigned char c, struct input_handler handler);
void register_render_component(void (*callback)(struct winsize ws));

#endif
