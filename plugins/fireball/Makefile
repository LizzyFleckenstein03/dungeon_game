plugins/fireball/fireball.so: plugins/fireball/fireball.c plugins/game/game.h plugins/movement/movement.h plugins/inventory/inventory.h
	cc -g -shared -fpic -o plugins/fireball/fireball.so plugins/fireball/fireball.c

PLUGINS := ${PLUGINS} plugins/fireball/fireball.so
