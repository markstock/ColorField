CC=gcc
CFLAGS=-O2 -march=native -Wall
LDFLAGS=-lm -lpng
EXE=colorfield

all : $(EXE)

% : %.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean :
	rm -f $(EXE)
