#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>

struct plugin_list
{
	char *name;
	void *handle;
	struct plugin_list *next;
};

struct plugin_list *plugins = NULL;
struct plugin_list **next = &plugins;

static void *load_plugin(const char *name)
{
	for (struct plugin_list *ptr = plugins; ptr != NULL; ptr = ptr->next) {
		if (strcmp(ptr->name, name) == 0)
			return ptr->handle;
	}

	size_t len = strlen(name);

	char dependency_file_name[1 + 1 + 7 + 1 + len + 1 + 12 + 1 + 3 + 1];
	sprintf(dependency_file_name, "./plugins/%s/dependencies.txt", name);

	FILE *dependency_file = fopen(dependency_file_name, "r");

	if (dependency_file) {
		char dependency[BUFSIZ];

		while (fscanf(dependency_file, "%s", dependency) != EOF)
			load_plugin(dependency);

		fclose(dependency_file);
	}

	char library_name[1 + 1 + 7 + 1 + len + 1 + len + 1 + 2 + 1];
	sprintf(library_name, "./plugins/%s/%s.so", name, name);

	void *handle = dlopen(library_name, RTLD_NOW | RTLD_GLOBAL);

	if (! handle) {
		printf("%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	char *namebuf = malloc(len + 1);
	strcpy(namebuf, name);

	*next = malloc(sizeof(struct plugin_list));
	**next = (struct plugin_list) {
		.name = namebuf,
		.handle = handle,
		.next = NULL,
	};
	next = &(*next)->next;

	printf("Loaded %s\n", name);

	return handle;
}

int main()
{
	void *main_plugin = load_plugin("game");

	DIR *dir = opendir("plugins");
	assert(dir);

	struct dirent *dp;

	while (dp = readdir(dir))
		if (dp->d_name[0] != '.')
			load_plugin(dp->d_name);

	closedir(dir);

	void (*game_func)() = dlsym(main_plugin, "game");
	assert(game_func);

	game_func();
}

