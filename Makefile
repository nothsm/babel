all: babel

babel: babel.c babel.h
	clang -Wall -Wconversion --std=c99 -o babel babel.c

clean:
	rm babel
