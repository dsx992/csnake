#ifndef RINGS
#define RINGS
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
 */
#define RING(NAME, TYPE, SIZE)		\
	struct NAME {			\
		unsigned int count;	\
		int head;		\
		int tail;		\
		TYPE data[SIZE];	\
	}
#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
#define RING_COUNT(RING) RING.count
#define RING_PUSH_TAIL(X, R) do {		\
	R.data[R.tail] = X;			\
	R.tail = (R.tail + 1) % LENGTH(R.data);	\
	if (R.count == LENGTH(R.data))			\
		R.head = (R.head + 1) % LENGTH(R.data);	\
	else					\
		R.count++;			\
} while (0)
#define RING_PUSH_HEAD(X, R) do {		\
	R.head = R.head - 1 < 0			\
		? LENGTH(R.data) - 1		\
		: (R.head - 1);			\
	R.data[R.head] = X;			\
	if (R.count == LENGTH(R.data))		\
		R.tail = R.tail - 1 < 0		\
			? LENGTH(R.data) - 1	\
			: (R.tail - 1);		\
	else					\
		R.count++;			\
} while (0)
#define RING_PEEK(I, R) (R.data[(R.head + I) % LENGTH(R.data)])
#define RING_PEEK_HEAD(R) (R.data[R.head])
#define RING_PEEK_TAIL(R)			\
	(R.data[R.tail - 1 < 0			\
	 	? LENGTH(R.data) - 1		\
	 	: R.tail - 1])
#define RING_REMOVE_HEAD(R) do {		\
	if (RING_COUNT(R) > 0) {		\
		R.head = (R.head + 1) % LENGTH(R.data);	\
		R.count--; 			\
	}					\
} while (0)
#define RING_REMOVE_TAIL(R) do {		\
	if (RING_COUNT(R) > 0) {		\
		R.tail = R.tail - 1 < 0		\
		? LENGTH(R.data) - 1		\
		: R.tail - 1;			\
		R.count--; 			\
	}					\
} while (0)

#endif
