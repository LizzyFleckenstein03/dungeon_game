#include <stdio.h>
#include <stdlib.h>
#include "../inventory/inventory.h"

static struct color gray;
static struct color darkgray;

static bool use_item(struct itemstack *stack)
{
	(void) stack;
	return true;
}

struct inventory player_inventory = {NULL};

static int selected_index = 0;

void inventory_add(struct inventory *self, struct itemstack stack)
{
	struct list **ptr = &self->stacks;

	if (stack.item->stackable) {
		for (; *ptr != NULL; ptr = &(*ptr)->next) {
			struct itemstack *other = (*ptr)->element;
			if (other->item == stack.item) {
				other->count += stack.count;
				return;
			}
		}
	}

	struct itemstack *buf = make_buffer(&stack, sizeof(struct itemstack));
	*ptr = add_element(*ptr, buf);

	if (buf->item->on_create)
		buf->item->on_create(buf);
}

/*

bool inventory_remove(struct inventory *self, struct itemstack *stack)
{
	stack.count = -stack.count;

	for (struct list **ptr = &self->stacks; *ptr != NULL; ) {
		struct itemstack *other = (*ptr)->element;

		if (other->item == stack.item) {
			stack.count += other->count;

			if (stack.count > 0) {
				other->count = stack.count;
				return true;
			} else {
				struct list *next = ptr->next;

				other->count = 0;

				if (other->item->on_destroy)
					other->item->on_destroy(other);

				free(other);
				free(*ptr);

				*ptr = next;
				continue;
			}
		}

		ptr = &(*ptr)->next;
	}

	return false;
}

*/

static void decrease_item_count(struct list **ptr, struct itemstack *stack)
{
	stack->count--;

	if (stack->count == 0) {
		struct list *next = (*ptr)->next;

		if (stack->item->on_destroy)
			stack->item->on_destroy(stack);

		if (stack->meta)
			free(stack->meta);

		free(stack);
		free(*ptr);

		*ptr = next;
	}
}

bool inventory_remove(struct inventory *self, struct item *item)
{
	for (struct list **ptr = &self->stacks; *ptr != NULL; ptr = &(*ptr)->next) {
		struct itemstack *stack = (*ptr)->element;

		if (stack->item == item) {
			decrease_item_count(ptr, stack);

			return true;
		}
	}

	return false;
}

static void handle_enter()
{
	int i = 0;

	for (struct list **ptr = &player_inventory.stacks; *ptr != NULL; ptr = &(*ptr)->next, i++) {
		if (i == selected_index) {
			struct itemstack *stack = (*ptr)->element;

			if (stack->item->on_use && stack->item->on_use(stack))
				decrease_item_count(ptr, stack);

			return;
		}
	}
}

static void handle_arrow()
{
	char c = fgetc(stdin);
	if (c == '[') {
		char dir = fgetc(stdin);

		int count = 0;

		for (struct list *ptr = player_inventory.stacks; ptr != NULL; ptr = ptr->next)
			count++;

		if (count == 0)
			return;

		switch (dir) {
			case 'A':
				selected_index--;

				if (selected_index < 0)
					selected_index = count - 1;
				break;
			case 'B':
				selected_index++;

				if (selected_index >= count)
					selected_index = 0;

				break;
		}
	} else {
		ungetc(c, stdin);
	}
}

static void render_inventory(struct winsize ws)
{
	printf("\e[3;0H");

	printf(" \e[1mInventory\e[21m\n");

	set_color(gray, false);

	int i = 0;
	for (struct list *ptr = player_inventory.stacks; ptr != NULL; ptr = ptr->next, i++) {
		struct itemstack *stack = ptr->element;

		if (i == selected_index) {
			printf(" \e[39m→ ");
			set_color(gray, false);
		} else {
			printf("   ");
		}

		printf("%s", stack->item->name);

		if (stack->count > 1) {
			set_color(darkgray, false);
			printf(" (x%u)", stack->count);
			set_color(gray, false);
		}

		printf("\n");
	}
}

__attribute__ ((constructor)) static void init()
{
	gray = get_color("#9E9E9E");
	darkgray = get_color("#555555");

	register_render_component(&render_inventory);

	register_input_handler('\033', (struct input_handler) {
		.run_if_dead = false,
		.callback = &handle_arrow,
	});

	register_input_handler('\n', (struct input_handler) {
		.run_if_dead = false,
		.callback = &handle_enter,
	});
}
