all: dungeon plugins

dungeon: dungeon.c
	cc -g -o dungeon dungeon.c -ldl

include plugins/*/Makefile

plugins: ${PLUGINS}
