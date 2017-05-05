CC=gcc
FLAGS=-Wall -ansi -pedantic

.PHONY: all
all: shell

shell: main.c error.o program.o job.o tokenizer.o built_in.o core.o
	$(CC) $(FLAGS) main.c error.o program.o job.o tokenizer.o built_in.o core.o -o shell

error.o: error.c error.h
	$(CC) -c $(FLAGS) error.c

program.o: program.c program.h
	$(CC) -c $(FLAGS) program.c

job.o: job.c job.h
	$(CC) -c $(FLAGS) job.c

tokenizer.o: tokenizer.c tokenizer.h
	$(CC) -c $(FLAGS) tokenizer.c

built_in.o: built_in.c built_in.h
	$(CC) -c $(FLAGS) built_in.c

core.o: core.c core.h
	$(CC) -c $(FLAGS) core.c

.PHONY: clean cleanobj
clean: cleanobj
	rm -f shell

cleanobj:
	rm -f *.o
