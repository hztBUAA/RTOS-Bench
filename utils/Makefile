include ../generator/Makefile
CC=gcc
PGMS=overhead
BASE_SRC=$(wildcard ../generator/*.c)
BASE_O=$(patsubst %.c,%.o,$(BASE_SRC))
override CFLAGS+=-O2 -Wall -g -I../generator -DGCC
CXXFLAGS=$(CFLAGS)

all: $(PGMS)
overhead:overhead.o $(BASE_O)
	$(CC) $(CFLAGS) $< $(BASE_SRC) -o $@ $(LDFLAGS)
clean:
	rm *.o $(PGMS) $(BASE_O)
