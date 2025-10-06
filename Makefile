all: babel

babel: babel.h babel.c Makefile
    clang -O0 -Wall -Wconversion -Werror --std=c99 -g -o babel babel.c

clean:
	rm babel && rm -rf babel.dSYM

