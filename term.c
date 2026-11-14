#include "term.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "point.h"
#include <signal.h>

volatile struct term_ctx term_ctx;

static struct termios term;

// When the window size changes, a SIGWINCH signal is sent to the foreground process group.
static void term_get_size()
{
	static struct winsize ws;
	ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
	term_ctx.w = ws.ws_col;
	term_ctx.h = ws.ws_row;
}

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

	term_get_size();
	/* TODO: signal */

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
