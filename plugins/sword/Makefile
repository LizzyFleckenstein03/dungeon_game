plugins/sword/sword.so: plugins/sword/sword.c plugins/game/game.h plugins/movement/movement.h plugins/inventory/inventory.h plugins/recharge/recharge.h plugins/loot/loot.h
	cc -g -shared -fpic -o plugins/sword/sword.so plugins/sword/sword.c

PLUGINS := ${PLUGINS} plugins/sword/sword.so
