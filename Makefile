# main: main.c
# 	gcc main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main 
# debug: main.c
# 	gcc -g main.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o main 
# 	gdb main
# run: main
# 	./main

# TODO: learn Make and have it check what platform we are compiling on

LFLAGS = `pkg-config --libs raylib`
CFLAGS += `pkg-config --cflags raylib`

main: main.c arena.h
	cc main.c $(LFLAGS) $(CFLAGS) -o main

.PHONY: all

all: main
	./main
