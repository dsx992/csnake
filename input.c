#include "input.h"
#include <pthread.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include "rings.h"
#include "rings.h"
#include <limits.h>
#include <stdlib.h>

#define RING_SIZE (2 << 8)
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static RING(, char, RING_SIZE) ring;

static struct termios term;
pthread_t tid;


static void init_term()
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
}

enum parse_state {
	PS_START,
	PS_LETTER,
	PS_ENTER,
	PS_ESCAPE,
	PS_CSI,
	PS_CSI_CMD,
	/* ignores and deletes unrecognized characters */
	PS_UNRECOGNIZED,
	/* ignores but does not delete. May be waiting for characters */
	PS_INVALID,
	/* Terminal */
	PS_END,
};

#define FORALL_ASCII_LETTERS	\
	X('A')			\
	X('B')			\
	X('C')			\
	X('D')			\
	X('E')			\
	X('F')			\
	X('G')			\
	X('H')			\
	X('I')			\
	X('J')			\
	X('K')			\
	X('L')			\
	X('M')			\
	X('N')			\
	X('O')			\
	X('P')			\
	X('Q')			\
	X('R')			\
	X('S')			\
	X('T')			\
	X('U')			\
	X('V')			\
	X('X')			\
	X('Y')			\
	X('Z')			\
	X('a')			\
	X('b')			\
	X('c')			\
	X('d')			\
	X('e')			\
	X('f')			\
	X('g')			\
	X('h')			\
	X('i')			\
	X('j')			\
	X('k')			\
	X('l')			\
	X('m')			\
	X('n')			\
	X('o')			\
	X('p')			\
	X('q')			\
	X('r')			\
	X('s')			\
	X('t')			\
	X('u')			\
	X('v')			\
	X('x')			\
	X('y')			\
	X('z')			\

/* This may be cursed.. I would like to replace it with some sort of loop,
 * ideally */
#define FORALL_ASCII_CHARS	\
	X(0)			\
	X(1)			\
	X(2)			\
	X(3)			\
	X(4)			\
	X(5)			\
	X(6)			\
	X(7)			\
	X(8)			\
	X(9)			\
	X(10)			\
	X(11)			\
	X(12)			\
	X(13)			\
	X(14)			\
	X(15)			\
	X(16)			\
	X(17)			\
	X(18)			\
	X(19)			\
	X(20)			\
	X(21)			\
	X(22)			\
	X(23)			\
	X(24)			\
	X(25)			\
	X(26)			\
	X(27)			\
	X(28)			\
	X(29)			\
	X(30)			\
	X(31)			\
	X(32)			\
	X(33)			\
	X(34)			\
	X(35)			\
	X(36)			\
	X(37)			\
	X(38)			\
	X(39)			\
	X(40)			\
	X(41)			\
	X(42)			\
	X(43)			\
	X(44)			\
	X(45)			\
	X(46)			\
	X(47)			\
	X(48)			\
	X(49)			\
	X(50)			\
	X(51)			\
	X(52)			\
	X(53)			\
	X(54)			\
	X(55)			\
	X(56)			\
	X(57)			\
	X(58)			\
	X(59)			\
	X(60)			\
	X(61)			\
	X(62)			\
	X(63)			\
	X(64)			\
	X(65)			\
	X(66)			\
	X(67)			\
	X(68)			\
	X(69)			\
	X(70)			\
	X(71)			\
	X(72)			\
	X(73)			\
	X(74)			\
	X(75)			\
	X(76)			\
	X(77)			\
	X(78)			\
	X(79)			\
	X(80)			\
	X(81)			\
	X(82)			\
	X(83)			\
	X(84)			\
	X(85)			\
	X(86)			\
	X(87)			\
	X(88)			\
	X(89)			\
	X(90)			\
	X(91)			\
	X(92)			\
	X(93)			\
	X(94)			\
	X(95)			\
	X(96)			\
	X(97)			\
	X(98)			\
	X(99)			\
	X(100)			\
	X(101)			\
	X(102)			\
	X(103)			\
	X(104)			\
	X(105)			\
	X(106)			\
	X(107)			\
	X(108)			\
	X(109)			\
	X(110)			\
	X(111)			\
	X(112)			\
	X(113)			\
	X(114)			\
	X(115)			\
	X(116)			\
	X(117)			\
	X(118)			\
	X(119)			\
	X(120)			\
	X(121)			\
	X(122)			\
	X(123)			\
	X(124)			\
	X(125)			\
	X(126)			\

static enum parse_state state_transitions[PS_END][CHAR_MAX] = {
#define X(CHAR) \
	[PS_START][CHAR] = PS_UNRECOGNIZED,
	FORALL_ASCII_CHARS
#undef X
#define X(LETTER) \
	[PS_START][LETTER] = PS_LETTER,
	FORALL_ASCII_LETTERS
#undef X
	[PS_START]['q'] = PS_LETTER,
	[PS_START]['Q'] = PS_LETTER,
	[PS_START]['\033'] = PS_ESCAPE,
	/* line feed */
	[PS_START][0xa] = PS_ENTER,
	/* carriage return */
	[PS_START][0xd] = PS_ENTER,
	/* PS_LETTER is a terminal */
#define X(CHAR) \
	[PS_LETTER][CHAR] = PS_END,
	FORALL_ASCII_CHARS
#undef X
#define X(CHAR) \
	[PS_ENTER][CHAR] = PS_END,
	FORALL_ASCII_CHARS
#undef X
	[PS_ESCAPE]['['] = PS_CSI,
	[PS_CSI]['A'] = PS_CSI_CMD,
	[PS_CSI]['B'] = PS_CSI_CMD,
	[PS_CSI]['C'] = PS_CSI_CMD,
	[PS_CSI]['D'] = PS_CSI_CMD,
	/* PS_CSI_CMD is a terminal */
#define X(CHAR) \
	[PS_CSI_CMD][CHAR] = PS_END,
	FORALL_ASCII_CHARS
#undef X
};

/*
 *	       [a-zA-Z]
 *	PS_START ---> PS_LETTER
 *	       \
 *	        \ '\033'
 *	         v        '['        [A-D]
 *	         PS_ESCAPE --> PS_CSI ---> PS_CSI_CMD
 */
static enum key parse_key()
{
	enum parse_state state = PS_START;
	enum key k = KEY_NULL;
	int r;
	char c;

	/* checks for empty, so going over bounds is OK. */
	for (r = 0; r < RING_COUNT(ring) + 1; r++) {
		c = RING_COUNT(ring) > 0 ? RING_PEEK(r, ring) : 0;
		state = state_transitions[state][(unsigned int)c];
		switch (state) {
			case PS_LETTER:
				switch (c) {
					case 'q':
					case 'Q':
						k = KEY_Q;
						break;
					default:
						break;
				}
				break;
			case PS_ENTER:
				k = KEY_ENTER;
				break;
			case PS_CSI_CMD:
				switch (c) {
					/* arrow keys are "\033[A" etc. */
					case 'A':
						k = KEY_UP;
						break;
					case 'B':
						k = KEY_DOWN;
						break;
					case 'D':
						k = KEY_LEFT;
						break;
					case 'C':
						k = KEY_RIGHT;
						break;
					default:
						break;
				}
				break;
			/* transitional */
			case PS_ESCAPE:
			case PS_CSI:
				break;
			/* exit states */
			case PS_UNRECOGNIZED:
			case PS_INVALID:
			case PS_END:
			default:
				goto done;
		}
	}
done:
	if (state == PS_END) {
		for (int i = 0; i < r; i++)
			RING_REMOVE_HEAD(ring);
	}

	if (state == PS_UNRECOGNIZED)
		RING_REMOVE_HEAD(ring);

	return k;
}

static void push(char c)
{
	pthread_mutex_lock(&lock);

	RING_PUSH_TAIL(c, ring);

	pthread_mutex_unlock(&lock);
}

static void *listener(void *args)
{
	char c;
	while(1) {
		c = getchar();
		push(c);
	}

	return NULL;
}


int input_init()
{
	int success;
	init_term();
	success = !pthread_create(&tid, NULL, &listener, NULL);

	return success;
}

void input_finish()
{
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &term);
	printf("\033[?1049l");
	/* hides cursor. ?25l to show */
	printf("\033[?25h");
	pthread_cancel(tid);

}

enum key input_get()
{
	enum key k;
	pthread_mutex_lock(&lock);

	k = parse_key();

	pthread_mutex_unlock(&lock);
	return k;
}
