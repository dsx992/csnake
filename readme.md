# snake

A c99 TUI game for unix terminals. Currently assume ANSI escape sequences and
`nanosleep()`.

# building

    $ make

# Structure

## drawing

drawing is done by moving the cursor with ANSI escape sequences.

## input

Input is continuously written to a ring by a thread that calls `getchar()`. A
request for a key press `input_get()` parses the head of the ring in terms of a
subset of escape codes (e.g. for arrow keys) by a finite state machine.
The state machine is implemented with arrays that work as maps. I have used X
macros extensively to populate the array.

# Project goal

I wanted a (fun) exercise in simple programming. The goal is an idiomatic C
implementation using classic tools (ctags, gdb, no lsp, no ai).

It is my conviction that design choices make up a non-trivial part of
programming.
Working in a simple environment (such as C) without any frameworks forces
critical thinking of such choices.

# links

- [setting the terminal](https://viewsourcecode.org/snaptoken/kilo/02.enteringRawMode.html)
- [ANSI escape codes](https://en.wikipedia.org/wiki/ANSI_escape_code)
- [macro data structure inspiration](https://man.openbsd.org/queue.3)

# licence

GNU PL with additional stipulation that repository will not be fed into a
training model.
