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
	const struct keymap *keymap;
	size_t size;
};

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static RING(, int) keys;
static struct listen_ctx listen_ctx;
pthread_t tid;


static int parse(const struct keymap *keymap, size_t size);
static void *listener(void *args);

int input_get()
{
	int k;

	if (RING_COUNT(keys)) {
		k = RING_PEEK_HEAD(keys);
		RING_REMOVE_HEAD(keys);
	} else {
		k = -1;
	}

	return k;
}
int input_getkey(const struct keymap *keymap, size_t size)
{
	int k = parse(keymap, size);
	return k;
}

int input_setreal(const struct keymap *keymap, size_t size)
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

static int parse(const struct keymap *keymap, size_t size)
{
	char buf[16];
	int count;
	int bind;
	int blocking;
	char c, p;

	bind = -1;
	count = 0;
	blocking = term_isblocking();

	term_setwaitkey(sizeof(buf) - 1);
	count = read(STDIN_FILENO, buf, sizeof(buf) - 1);
	term_setnowaitkey();
	buf[count++] = 0;

	for (int m = 0; m < size && bind < 0 && count; m++) {
		for (int i = 0; i < count ; i++) {
			p = keymap[m].pattern[i];
			c = buf[i];

			if (p == '\0' && c == 0) {
				bind = keymap[m].bind;
				break;
			}

			if (p != c)
				break;
		}
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

		if ((k = parse(listen_ctx.keymap, listen_ctx.size)) != -1)
			RING_PUSH_TAIL(k, keys);

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}

	return NULL;
}
