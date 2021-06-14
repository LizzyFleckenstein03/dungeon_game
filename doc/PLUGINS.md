# Plugins

You can easily create plugins for the game by putting a new folder into plugins/ with the name of your plugin and then compiling your code into a shared library placed inside this folder named "<plugin name>.so" (`-shared -fpic`).
You might want to include the game.h file from plugins/game/game.h. Have a look into it to see available API. See the existing plugins for examples.

## Dependencies

If you want to make a plugin that needs to use ABI from another plugin (including the game itself), make sure to depend on that plugin. To add dependencies to a plugin, create a file named dependencies.txt in the plugin folder. Put the names of all plugins your plugin depends on into that file. You can use spaces or newlines as seperators.

## Makefile inclusion
All Makefiles that are placed in plugin directories are included by the main Makefile, so you might want to include a makefile in your plugin. The plugins target simply depends on ${PLUGINS}, so just add things to this in your plugin Makefile to add them to the plugins target (usually your plugin.so)
