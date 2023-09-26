GCC_FLAGS = ../utils/heap_help/heap_help.c -ldl -rdynamic -Wextra -Werror -Wall -Wno-gnu-folding-constant

all: libcoro.c solution.c
	gcc $(GCC_FLAGS) libcoro.c solution.c mine.c -o main

clean:
	rm main
