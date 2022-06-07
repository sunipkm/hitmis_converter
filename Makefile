CC = gcc
EDCFLAGS = -O2 -I ./ `pkg-config cfitsio --cflags`
EDLDFLAGS = -lcfitsio -lm `pkg-config cfitsio --libs`
COBJS = main.o

all: $(COBJS)
	$(CC) -o convert.out $(COBJS) $(EDLDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(EDCFLAGS)

.PHONY: clean

clean:
	rm -vf *.o
	rm -vf *.out
	rm -vf *.fit
