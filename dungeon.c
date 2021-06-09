#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <assert.h>
#include <dirent.h>
#include <string.h>

static void *load_plugin(const char *name)
{
	size_t len = strlen(name);
	char filename[1 + 1 + 7 + 1 + len + 1 + len + 1 + 2 + 1];
	sprintf(filename, "./plugins/%s/%s.so", name, name);

	void *plugin_handle = dlmopen(LM_ID_BASE, filename, RTLD_NOW | RTLD_GLOBAL);

	if (! plugin_handle) {
		printf("%s\n", dlerror());
		exit(EXIT_FAILURE);
	}

	return plugin_handle;
}

int main()
{
	void *main_plugin = load_plugin("game");

	DIR *dir = opendir("plugins");
	assert(dir);

	struct dirent *dp;

	while (dp = readdir(dir)) {
		if (dp->d_name[0] != '.' && strcmp(dp->d_name, "game") != 0) {
			load_plugin(dp->d_name);
		}
	}

	closedir(dir);

	void (*game_func)() = dlsym(main_plugin, "game");
	assert(game_func);

	game_func();
}

