GENERATES = main.out
MY_OBG_FILES = *.o
CC = g++
CPPFLAGS = -fconcepts-diagnostics-depth=2 -fsanitize=address,undefined,signed-integer-overflow,pointer-compare,pointer-subtract,leak,bounds,pointer-overflow -O2 -Wall -Wextra -Wpedantic -std=c++23 -lm


.PHONY: all run clean

all: main.out

run: main.out
	./main.out

test: tests.out
	./tests.out

tests.out: tests.cpp funclib.cpp
	$(CC) tests.cpp -O2 -Wall -Wextra -Wpedantic -std=c++23 -lm -lgtest -o tests.out

main.out: main.o
	$(CC) $(CPPFLAGS) -o $@ $^

main.o: main.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $^

objclean:
	rm -v $(MY_OBG_FILES)

distclean:
	rm -v $(GENERATES)

clean: objclean distclean
