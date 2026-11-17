#ifndef RINGS
#define RINGS

#include "arena.h"
#include "stddef.h"

/*
 * Ring datastructures.
 * `NAME` is the name of the resulting struct,
 * `TYPE` is the type of the data.
 *
 * The header exposes macros `RING()` to create a new type,
 * `RING_PUSH()` to push an element to a ring,
 * `RING_PEEK_HEAD()` to get the element at the head,
 * `RING_REMOVE_HEAD()` to pop (without getting back a value) the head,
 * and `RING_COUNT()` to get the number of elements in a ring.
 *
 * notes:
 * 	The method is loosely based on openbsds macro datastructures,
 * 	specifically `RING(name, type) ring;` is supposed to work. Therefore
 * 	everything is assumed to be initialized to 0.
 * 	The ring is overwriting. This means that pushing to a full ring moves
 * 	the head.
 *
 * TODO: the TYPE data[SIZE] construct is not flexible enough, e.g., not
 * compile-time game dimensions.
 * TODO: I think maybe an arena allocator would be good?
 */
#define RING(NAME, TYPE)		\
	struct NAME {			\
		TYPE *data;		\
		size_t size;		\
		unsigned int count;	\
		int head;		\
		int tail;		\
	}
#define RING_INITIALIZE(RING, SIZE) do {	\
	RING.data = arena_malloc(sizeof(*(RING.data)) * SIZE);	\
	RING.size = SIZE;			\
	RING.count = 0;				\
	RING.head = 0;				\
	RING.tail = 0;				\
} while (0)
#define RING_COUNT(RING) RING.count
#define RING_PUSH_TAIL(X, R) do {		\
	R.data[R.tail] = X;			\
	R.tail = (R.tail + 1) % R.size;	\
	if (R.count == R.size)			\
		R.head = (R.head + 1) % R.size;	\
	else					\
		R.count++;			\
} while (0)
#define RING_PUSH_HEAD(X, R) do {		\
	R.head = R.head - 1 < 0			\
		? R.size - 1		\
		: (R.head - 1);			\
	R.data[R.head] = X;			\
	if (R.count == R.size)		\
		R.tail = R.tail - 1 < 0		\
			? R.size - 1	\
			: (R.tail - 1);		\
	else					\
		R.count++;			\
} while (0)
#define RING_PEEK(I, R) (R.data[(R.head + I) % R.size])
#define RING_PEEK_HEAD(R) (R.data[R.head])
#define RING_PEEK_TAIL(R)			\
	(R.data[R.tail - 1 < 0			\
		? R.size - 1			\
	 	: R.tail - 1])
#define RING_REMOVE_HEAD(R) do {		\
	if (RING_COUNT(R) > 0) {		\
		R.head = (R.head + 1) % R.size;	\
		R.count--; 			\
	}					\
} while (0)
#define RING_REMOVE_TAIL(R) do {		\
	if (RING_COUNT(R) > 0) {		\
		R.tail = R.tail - 1 < 0		\
		? R.size - 1		\
		: R.tail - 1;			\
		R.count--; 			\
	}					\
} while (0)
#define WRITETO(BUF, R)				\
	for (int i = 0; i < RING_COUNT(R); i++)	\
		BUF[i] = RING_PEEK(i, R);	\

#endif
