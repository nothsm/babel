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
	clang -g -Wall -Wconversion --std=c99 -o babel babel.c

babel-leaks: babel.c babel.h
	clang -fsanitize=address -g -Wall -Wconversion --std=c99 -o babel-leaks babel.c

engine: engine.c nn.c main.c babel.h
	clang -g -Wall -Wconversion --std=c99 -o main engine.c nn.c main.c

engine-leakcheck: engine.c nn.c babel.h
	clang -fsanitize=address -g -Wall -Wconversion --std=c99 -o engine-leakcheck engine.c nn.c && \
	MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./engine-leakcheck

test: engine.c nn.c babel.h test.c test.h
	clang -g -v -Wall -Wconversion --std=c99 -o test test.c engine.c nn.c

clean:
	rm -f babel;              \
	rm -rf babel.dSYM;        \
	rm -f babel-leaks;        \
	rm -rf babel-leaks.dSYM;  \
	rm -f main;             \
	rm -rf main.dSYM;       \
	rm -f engine-leakcheck;       \
	rm -rf engine-leakcheck.dSYM; \
	rm -f test;               \
	rm -rf test.dSYM;
