#include "input.h"
#include <pthread.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include "rings.h"
#include "rings.h"
#include <limits.h>
#include <stdlib.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static RING(, char, 2 << 8) ring;

pthread_t tid;

static void *listener(void *args);
static void push(char c);
enum key key_parse();

int input_init()
{
	return !pthread_create(&tid, NULL, &listener, NULL);
}

void input_finish()
{
	pthread_cancel(tid);

}

enum key input_get_key()
{
	enum key k;
	pthread_mutex_lock(&lock);

	k = key_parse();

	pthread_mutex_unlock(&lock);
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

enum key key_parse()
{
	/* the ring type is flexible enough (it is bidirectional) to work as a
	 * stack */
	char c, p;
	int i, m;

	for (m = 0; m < LENGTH(keymap); m++) {
		for (i = 0; (p = keymap[m].pattern[i]); i++) {
			if (RING_COUNT(ring) == 0)
				return KEY_INVALID;

			c = RING_PEEK(i, ring);
			/* input does not match pattern */
			if (c != p)
				break;
		}

		/* we consumed a production = success */
		if (!p) {
			while (i--)
				RING_REMOVE_HEAD(ring);
			return keymap[m].key;
		}
	}
	
	/* unkonwn key, so remove the first character */
	RING_REMOVE_HEAD(ring);

	return KEY_UNKNOWN;
}
