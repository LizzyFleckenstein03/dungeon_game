# Building

The loader needs GCC to compile (`__USE_GNU`).

To build the loader and the plugins in the plugins/ folder, simply type `make` or `make all`. There are separate targets for the loader (`dungeon`) and the plugins.
To run the loader, type `./dungeon`. It will load all plugins including the game itself dynamically and run the game.

