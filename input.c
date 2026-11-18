#include "input.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "rings.h"
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include "common.h"

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static RING(, char) ring;

pthread_t tid;

static void *listener(void *args);
enum key key_parse();

/* TODO: add debounce */

/* should not be used when input_init */
int input_getchar()
{
	char c = getchar();
	RING_PUSH_TAIL(c, ring);
	return 1;
}

int input_raw()
{
	return !pthread_create(&tid, NULL, &listener, NULL);
}

void input_blocking()
{
	pthread_cancel(tid);
}

int input_init()
{
	RING_INITIALIZE(ring, 2 << 8);
	return ring.data != NULL;
}

enum key input_get_key()
{
	enum key k;
	pthread_mutex_lock(&lock);

	k = key_parse();

	pthread_mutex_unlock(&lock);
	return k;
}

static void *listener(void *args)
{
	char c;
	while(1) {
		c = getchar();
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock(&lock);

		RING_PUSH_TAIL(c, ring);

		pthread_mutex_unlock(&lock);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}

	return NULL;
}

enum key key_parse()
{
	/* the ring type is flexible enough (it is bidirectional) to work as a
	 * stack */
	char c, p;
	int i, m, u;

	u = 0;
	for (m = 0; m < LENGTH(keymap); m++) {
		for (i = 0; (p = keymap[m].pattern[i]); i++) {
			if (RING_COUNT(ring) - i <= 0) {
				u = 1;
				break;
			}

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

	/* if at least one production stopped unexpectedly, wait for more */
	if (u)
		return KEY_INVALID;

	/* unkonwn key, so remove the first character */
	RING_REMOVE_HEAD(ring);

	return KEY_UNKNOWN;
}
