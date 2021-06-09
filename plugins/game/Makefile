plugins/game/game.so: plugins/game/game.c plugins/game/game.h
	cc -g -shared -fpic -o plugins/game/game.so plugins/game/game.c -lm -lpthread

PLUGINS := ${PLUGINS} plugins/game/game.so
