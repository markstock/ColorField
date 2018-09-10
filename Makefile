CC=gcc
CFLAGS=-O3 -ffast-math -Wall
LDFLAGS=-lm -lpng
EXE=colorfield

all : $(EXE)

% : %.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

clean :
	rm -f $(EXE)
