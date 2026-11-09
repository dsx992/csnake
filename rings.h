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

#define RING_COUNT(RING) RING.count
#define RING_PUSH(X, R) do {			\
	size_t sz = sizeof(R.data) / sizeof(R.data[0]); \
	R.data[R.tail] = X;			\
	R.tail = (R.tail + 1) % sz;		\
	if (R.count == sz)			\
		R.head = (R.head + 1) % sz;	\
	else					\
		R.count++;			\
} while (0)
#define RING_PEEK_HEAD(R) (RING_COUNT(R) == 0 ? 0 : R.data[R.head])
#define RING_REMOVE_HEAD(R) do {		\
	size_t sz = sizeof(R.data) / sizeof(R.data[0]); \
	if (RING_COUNT(R) > 0) {		\
		R.head = (R.head + 1) % sz;	\
		R.count--; 			\
	}					\
} while (0)

#endif
