CC=mpicc
DEPS = list.h
DEPS_C = main.c list.c
TARGETS = main

all : $(TARGETS)

main: $(DEPS_C) $(DEPS)
	$(CC)  $^ $(CFLAGS) -o $@

clean:
	rm -f $(TARGETS) *.o

