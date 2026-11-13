#include "term.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static struct termios term;

int term_init()
{
	struct termios raw;

	tcgetattr(STDIN_FILENO, &term);

	raw = term;
	raw.c_lflag &= ~(ECHO | ICANON);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
	/* CSI ? 1049 h
	 * CSI ? 1049 l
	 */
	/* alternate buffer */
	printf("\033[?1049h");
	/* hides cursor. ?25h to show */
	printf("\033[?25l");

	return 1;
}

int term_finish()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
	printf("\033[?1049l");
	/* hides cursor. ?25l to show */
	printf("\033[?25h");

	return 1;
}
