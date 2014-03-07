
all: bootstrap

bootstrap: lisp.c lisp.h
	$(CC) -g -o $@ $< -std=c99 -Wall -Werror

.PHONY clean:
	-rm bootstrap
