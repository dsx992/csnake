/*
 * a collection of some macros and defines that were copied to most files
 */

#define _POSIX_C_SOURCE 199309L

#define LENGTH(X) (sizeof(X) / sizeof(*X))
#define MAX(X, Y) ((X) > (Y) ? (X) : (Y))
#define MIN(X, Y) ((X) < (Y) ? (X) : (Y))
