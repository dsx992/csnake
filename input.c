#include "common.h"
#include "term.h"
#include "input.h"
#include "rings.h"
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct listen_ctx {
	int running;
	struct keymap *keymap;
	size_t size;
};

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static RING(, int) keys;
static struct listen_ctx listen_ctx;
pthread_t tid;


static int parse(struct keymap *keymap, size_t size);
static void *listener(void *args);

int input_get()
{
	int k;

	pthread_mutex_lock(&lock);

	if (RING_COUNT(keys)) {
		k = RING_PEEK_HEAD(keys);
		RING_REMOVE_HEAD(keys);
	} else {
		k = -1;
	}

	pthread_mutex_unlock(&lock);

	return k;
}
int input_getkey(struct keymap *keymap, size_t size)
{
	return parse(keymap, size);
}

int input_setreal(struct keymap *keymap, size_t size)
{
	if (listen_ctx.running)
		return 0;

	/* flush */
	while (RING_COUNT(keys))
		RING_REMOVE_HEAD(keys);

	listen_ctx.keymap = keymap;
	listen_ctx.size = size;
	listen_ctx.running = 1;

        return !pthread_create(&tid, NULL, &listener, NULL);
}

int input_setnoreal()
{
	if (!listen_ctx.running)
		return 0;

	pthread_cancel(tid);

	/* flush */
	while (RING_COUNT(keys))
		RING_REMOVE_HEAD(keys);

	listen_ctx.running = 0;

	return 1;
}

int input_init()
{
	RING_INITIALIZE(keys, 2 << 8);
	listen_ctx.running = 0;

	return keys.data != NULL;
}

static int parse(struct keymap *keymap, size_t size)
{
	char buf[16];
	int count = 0;
	int m, i, bind;
	int blocking;
	char c, p;

	bind = -1;
	blocking = term_isblocking();
	c = getchar();
	buf[count++] = c;

	term_setnotblocking();

	for (m = 0; m < size && (bind < 0); m++) {
		i = 0;
		for (i = 0; ; i++) {
			p = keymap[m].pattern[i];
			/* if we are waiting for a new character */
			if (i >= count)
				buf[count++] = getchar();
			/* if we timed out */
			if (buf[i] == EOF)
				break;
			if (p == '\0')
				break;
			if (p != buf[i])
				break;

		}

		/* we expect to always time out between keystrokes */
		if (buf[i] == EOF && p == )

		if (i == count && c == EOF && p == '\0')
			bind = keymap[m].bind;
		

		// for (i = 0; i < count; i++) {
		// 	p = keymap[m].pattern[i];
		// 	if (p == '\0')
		// 		break;
		// 	if (p != buf[i])
		// 		break;
		// }
		// c = getchar();
		// /* key sequence over, success */
		// if (i == count && c == EOF && p == '\0')
		// 	bind = keymap[m].bind;
		// buf[count++] = getchar();
	}

	if (blocking)
		term_setblocking();

	return bind;
}

static void *listener(void *args)
{
	int k;

	while(1) {
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock(&lock);

		if ((k = parse(listen_ctx.keymap, listen_ctx.size)) != 1)
			RING_PUSH_TAIL(k, keys);

		pthread_mutex_unlock(&lock);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}

	return NULL;
}
