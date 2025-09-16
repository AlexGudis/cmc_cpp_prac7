GENERATES = main.out
OBG_FILES = *.o
TRASH = logs/*.log
CC = g++
CPPFLAGS = -fconcepts-diagnostics-depth=2 -fsanitize=address,undefined,signed-integer-overflow,pointer-compare,pointer-subtract,leak,bounds,pointer-overflow -O2 -Wall -Wextra -Wpedantic -std=c++23 -lm


.PHONY: all run clean

all: main.out

run: main.out
	./main.out

main.out: main.o shr_ptr.o
	$(CC) $(CPPFLAGS) -o $@ $^

main.o: main.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

shared_pointer.o: shr_ptr.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

objclean:
	rm -v $(OBG_FILES)

distclean:
	rm -v $(GENERATES)

trashclean:
	rm -v $(TRASH)

clean: objclean distclean trashclean
