CC = gcc
FLAGS = -std=c99 -fopenmp -Ofast -Wall -Werror

all: qs-psrs

qs-psrs: qs-psrs.c
	$(CC) $(FLAGS) $^ -o $@

clean:
	rm -f qs-psrs *.o