CC = gcc
EDCFLAGS = -O2 -I ./
EDLDFLAGS = -lcfitsio -lm
COBJS = main.o

all: $(COBJS)
	$(CC) -o test.out $(COBJS) $(EDLDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(EDCFLAGS)

.PHONY: clean

clean:
	rm -vf *.o
	rm -vf *.out
	rm -vf tmp.fit