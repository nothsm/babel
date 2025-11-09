all: babel

interact: babel
	./babel

filter: babel
	./babel -f

babel: babel.c babel.h
	clang -g -Wall -Wconversion --std=c99 -o babel babel.c

filter-leaks: babel-leaks
	MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./babel-leaks -f

interact-leaks: babel-leaks
	MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./babel-leaks

babel-leaks: babel.c babel.h
	clang -fsanitize=address -g -Wall -Wconversion --std=c99 -o babel-leaks babel.c

engine: engine.c babel.h
	clang -g -Wall -Wconversion --std=c99 -o engine engine.c

engine-leaks: engine.c engine.h
	clang -fsanitize=address -g -Wall -Wconversion --std=c99 -o engine-leaks engine.c && MallocNanoZone=0 ASAN_OPTIONS=detect_leaks=1 ./engine-leaks

clean:
	rm babel; rm babel-leaks; rm -rf babel.dSYM; rm -rf babel-leaks.dSYM; rm engine; rm engine-leaks; rm -rf engine.dSYM; rm -rf engine-leaks.dSYM
