all: dungeon plugins

dungeon: dungeon.c
	cc -g -o dungeon dungeon.c -ldl -D_GNU_SOURCE

include plugins/*/Makefile

plugins: ${PLUGINS}
