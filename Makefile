OPT=-O3 -Wall
PROG=genrandom

all: $(PROG)

$(PROG): $(PROG)_splitmix64.c
	$(CC) $(LDFLAGS) $(CFLAGS) $(OPT) -o $@ $< $(LIBS)

clean:
	rm -f *.o $(PROG)

