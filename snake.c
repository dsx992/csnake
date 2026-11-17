#define _POSIX_C_SOURCE 2

#include "arena.h"
#include "game.h"
#include "input.h"
#include "menu.h"
#include "term.h"
/* getopt */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

struct game_ctx;
extern struct game_ctx game_ctx;
extern char *optarg;

static enum menuitem (*menus[MENUITEM_END])() = {
	[MENUITEM_MENU] = &menu,
	[MENUITEM_PLAY] = &game,
	[MENUITEM_POST] = &postgame,
	[MENUITEM_SCORE] = &leaderboard,
	[MENUITEM_HELP] = &help,
};

static void usage();
static void failure();
static char *prog;

int main(int argc, char *argv[])
{
	enum menuitem m;
	int opt;

	prog = argv[0];

	while ((opt = getopt(argc, argv, "w:h:s:")) != -1) {
		switch (opt) {
			case 'w':
				game_ctx.w = atoi(optarg);
				printf("game width: %i\n", game_ctx.w);
				if (game_ctx.w < 0)
					failure();
				break;
			case 'h':
				game_ctx.h = atoi(optarg);
				if (game_ctx.h < 0)
					failure();
				break;
			case 's':
				game_ctx.tickrate = atoi(optarg);
				if (game_ctx.tickrate < 0)
					failure();
				break;
			default:
				failure();
		}
	}

	arena_init(1<<16);
	term_init();
	input_init();
	initialize_game_ctx();

	m = MENUITEM_MENU;

	while (menus[m])
		m = menus[m]();

	term_finish();
	arena_finish();
	return 0;
}

static void usage(FILE *std)
{
	fprintf(std, "Usage: %s "
			"[-w width] [-h height] [-s tickrate]\n"
			, prog);
}

static void failure()
{
	usage(stderr);
	exit(EXIT_FAILURE);
}
