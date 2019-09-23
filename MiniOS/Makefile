CC = gcc
FLAGS = -g -O2 -Wall -Wextra -std=c99
EXE = tema2

.PHONY: build clean

build: $(EXE)

$(EXE): main.o list.o stack.o queue.o
	$(CC) $(FLAGS) main.o list.o stack.o queue.o -o $(EXE)

main.o: main.c stack.h queue.h
	$(CC) $(FLAGS) main.c -c -o main.o

list.o: list.c list.h
	$(CC) $(FLAGS) list.c -c -o list.o

stack.o: stack.c stack.h list.h
	$(CC) $(FLAGS) stack.c -c -o stack.o

queue.o: queue.c queue.h list.h
	$(CC) $(FLAGS) queue.c -c -o queue.o

clean:
	rm $(EXE) main.o list.o stack.o queue.o
