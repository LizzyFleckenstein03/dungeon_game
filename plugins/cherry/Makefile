plugins/cherry/cherry.so: plugins/cherry/cherry.c plugins/game/game.h plugins/score/score.h plugins/inventory/inventory.h plugins/loot/loot.h
	cc -g -shared -fpic -o plugins/cherry/cherry.so plugins/cherry/cherry.c

PLUGINS := ${PLUGINS} plugins/cherry/cherry.so
