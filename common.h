/*
 * a collection of some macros and defines that were copied to most files
 */

#define _POSIX_C_SOURCE 199309L

#define LENGTH(X) (sizeof(X) / sizeof(*X))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))

#define LOCK(MUT) do {\
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); \
	pthread_mutex_lock(&MUT); \
} while (0)
#define UNLOCK(MUT) do {\
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); \
	pthread_mutex_unlock(&MUT); \
} while (0)
