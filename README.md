# Dungeon Game
A small but flexible dungeon crawler written in C. Needs GCC to compile (`__USE_GNU`). Renders directly into the terminal.

You can easily create plugins for the game by putting a new folder into plugins/ with the name of your plugin and then compiling your code into a shared library placed inside this folder named "<plugin name>.so" (`-shared -fpic`).
You might want to include the game.h file from plugins/game/game.h. Have a look into it to see available API. See the existing plugins for examples.

Controls: WASD to move, Q to quit.

To build the loader and the plugins in the plugins/ folder, simply type `make` or `make all`. There are separate targets for the loader (`dungeon`) and the plugins. The plugins target simply triggerst all Makefiles that are placed in plugin directories, so you might want to include a makefile in your plugin.
To run the loader, type `./dungeon`. It will load all plugins including the game itself dynamically and runs the game.

Plugins are loaded in alphabethical order, with the exception of the game plugin that is loaded first. If you want to make a plugin that depends on another plugin, make sure the other plugin is loaded first by setting the name of your plugin accordingly. A cleaner solution to this is coming soon.
