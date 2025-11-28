all: babel

run-interact: babel
	./babel

run-filter: babel
	./babel -f

run-interact-leaks: babel-leaks
	MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./babel-leaks

run-filter-leaks: babel-leaks
	MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./babel-leaks -f

babel: babel.c babel.h
	clang -g -Wall -Wconversion -Werror --std=c99 -o babel babel.c

babel-leaks: babel.c babel.h
	clang -fsanitize=address -g -Wall -Wconversion -Werror --std=c99 -o babel-leaks babel.c

main: engine.c nn.c main.c babel.h
	clang -g -Wall -Wconversion -Werror --std=c99 -o main engine.c nn.c main.c

main-leakcheck: engine.c nn.c babel.h
	clang -fsanitize=address -g -Wall -Wconversion -Werror --std=c99 -o main-leakcheck engine.c nn.c main.c && \
	MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./main-leakcheck

test: engine.c nn.c babel.h test.c test.h
	clang -g -Wall -Wconversion -Werror --std=c99 -o test test.c engine.c nn.c

clean:
	rm -f babel;                \
	rm -rf babel.dSYM;          \
	rm -f babel-leaks;          \
	rm -rf babel-leaks.dSYM;    \
	rm -f main;                 \
	rm -rf main.dSYM;           \
	rm -f main-leakcheck;       \
	rm -rf main-leakcheck.dSYM; \
	rm -f test;                 \
	rm -rf test.dSYM;
