#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include "input.h"
#include <time.h>
#include <stdlib.h>

#define WIDTH 25
#define HEIGHT 25

struct point{
	int x, y;
};

const struct timespec rqtp = {
	.tv_sec = 0,
	/* 20 fps -> 1 / 20 seconds -> (1 / 20) / 1'000'000'000 nanoseconds
	 * -> 1'000'000'000 / 20 -> 100'000'000 / 2 */
	.tv_nsec = 100000000 / 2,
};

void clear_screen()
{
	printf("\033[2J");
}

struct point snake = {.x = 10, .y = 10};

int handle_key(char);

int main()
{
	char c;
	input_init();
	atexit(&input_finish);

	clear_screen();

	while(1) {
		c = input_get();
		if (!handle_key(c))
			break;

		printf("\033[%i;%iH", snake.y, snake.x * 2);
		printf("[]");
		fflush(stdout);
		nanosleep(&rqtp, NULL);
	}

	return 0;
}

int handle_key(char c)
{
	switch (c) {
		case 'q':
			return 0;
		case 'h':
			snake.x--;
			break;
		case 'j':
			snake.y++;
			break;
		case 'k':
			snake.y--;
			break;
		case 'l':
			snake.x++;
			break;
		default:
			break;
	}
	return 1;
}
