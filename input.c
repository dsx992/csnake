#include "input.h"
#include <pthread.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

/*
 * Overwriting ring buffer.
 * reads from head + 1.
 * writes to tail.
 * is empty if head + 1 == tail.
 */
#define RING_SIZE (2 << 3)
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int head = -1;
static int tail = 0;
static char data[RING_SIZE];

static struct termios term;
pthread_t tid;

static int incr(int x)
{
	return (x + 1) % RING_SIZE;
}

static int empty()
{
	return incr(head) == tail;
}


static void push(char c)
{
	pthread_mutex_lock(&lock);

	/* ring is full */
	if (tail == head)
		head = incr(head);

	data[tail] = c;
	tail = incr(tail);

	pthread_mutex_unlock(&lock);
}

static char pop()
{
	char c;
	pthread_mutex_lock(&lock);

	if (empty())
		c = 0;
	else
		c = data[head = incr(head)];

	pthread_mutex_unlock(&lock);
	return c;
}


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
	printf("\033[?1049h");
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
	pthread_cancel(tid);

}

char input_get()
{
	return pop();
}
