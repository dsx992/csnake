#include "common.h"
#include "input.h"
#include "term.h"
#include "rings.h"
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

/* in nanoseconds. 1 second = 1'000'000'000 */
#define TIMEOUT 50000L

static int key_block = 1;
static const struct keymap *keymap;
static size_t keymap_size;

static struct timespec last_capture;
static RING( , char) ring;
static int seqcount = 0;
static int capturing = 0;
static pthread_t capture_tid, timeout_tid;
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t captured_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t keypress_cond = PTHREAD_COND_INITIALIZER;

static int input_popseq(char *to, size_t size);
static void *capture_thread(void *args);
static void *timeout_thread(void *args);

void input_flush()
{
	LOCK(mut);
	/* wait until a capture is finished */
	if (capturing)
		pthread_cond_wait(&captured_cond, &mut);

	while (RING_COUNT(ring))
		RING_REMOVE_HEAD(ring);

	seqcount = 0;

	UNLOCK(mut);
}

void input_setblocking(int b)
{
	key_block = b;
}

void input_setkeymap(const struct keymap *km, size_t ks)
{
	keymap = km;
	keymap_size = ks;
}

/*
 * If no keymap or no keymapping matching the sequence, returns the first
 * character of the sequence as
 * 	(int)(c ^ 0xff)
 * or -128 if no input were read.
 */
int input_getkey()
{
	static char buf[16];
	int ok = 1;

	LOCK(mut);
	/* captured_cond is broadcast, so the mutex may be locked elsewhere,
	 * and seqcount may decrease */
	while (key_block && (seqcount == 0))
		pthread_cond_wait(&captured_cond, &mut);

	ok = input_popseq(buf, sizeof(buf));

	UNLOCK(mut);

	if (!ok)
		return (int)((signed char)0xff);

	if (!keymap)
		return (int)(buf[0] ^ 0xff);

	for (int i = 0; i < keymap_size; i++)
		if (strcmp(keymap[i].string, buf) == 0)
			return keymap[i].bind;

	return (int)(buf[0] ^ 0xff);
}

void input_init()
{
	RING_INITIALIZE(ring, 2 << 6);
	clock_gettime(CLOCK_REALTIME, &last_capture);
	pthread_create(&capture_tid, NULL, &capture_thread, NULL);
	pthread_create(&timeout_tid, NULL, &timeout_thread, NULL);
}

void input_finish()
{
	pthread_cancel(capture_tid);
	pthread_cancel(timeout_tid);
}

static void *capture_thread(void *args)
{
	char c;
	while (1) {
		read(STDIN_FILENO, &c, 1);

		LOCK(mut);

		RING_PUSH_TAIL(c, ring);
		clock_gettime(CLOCK_REALTIME, &last_capture);
		pthread_cond_signal(&keypress_cond);

		UNLOCK(mut);
	}
	return NULL;
}

static void *timeout_thread(void *args)
{
	struct timespec now, interval;

	while (1) {
		/* key sequence timed out, waiting for the start of another, as
		 * signaled by  `capture_thread()` */
		LOCK(mut);
		if (!capturing) {
			pthread_cond_wait(&keypress_cond, &mut);
			capturing = 1;
		}

		/* how long to sleep. */
		clock_gettime(CLOCK_REALTIME, &now);
		interval.tv_nsec = now.tv_sec - last_capture.tv_sec
			? TIMEOUT
			: now.tv_nsec - last_capture.tv_nsec;
		interval.tv_sec = 0;

		UNLOCK(mut);

		nanosleep(&interval, NULL);

		LOCK(mut);

		clock_gettime(CLOCK_REALTIME, &now);

		interval.tv_sec = now.tv_sec - last_capture.tv_sec;
		interval.tv_nsec = now.tv_nsec - last_capture.tv_nsec;

		/* if no timeout, fine */
		if (interval.tv_nsec < TIMEOUT)
			continue;

		/* we stopped capturing, so timeout_thread will announce that
		 * capturing has stopped and wait for keypress_cond to continue
		 * */
		capturing = 0;
		seqcount++;
		pthread_cond_broadcast(&captured_cond);

		RING_PUSH_TAIL('\0', ring);

		UNLOCK(mut);
	}
	return NULL;
}

static int input_popseq(char *to, size_t size)
{
	size_t i;

	if (RING_COUNT(ring) == 0)
		return 0;

	for (i = 0; i < RING_COUNT(ring) && i < size; i++) {
		to[i] = RING_PEEK(i, ring);
		if (to[i] == '\0')
			break;
	}

	/* truncation; do not remove from ring */
	if ((RING_COUNT(ring) == 0) || (i == size)) {
		to[i - 1] = '\0';
		exit(67);
		return 0;
	}

	/* success; remove sequence from ring */
	for (int j = 0; j <= i; j++)
		RING_REMOVE_HEAD(ring);

	seqcount--;

	return 1;
}
