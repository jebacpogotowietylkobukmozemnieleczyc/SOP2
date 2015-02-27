CC=gcc
CFLAGS=-Wall -I.
DEPS = inf117244_sdata.h
OBJ = inf117244_s.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

inf117244_s: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)