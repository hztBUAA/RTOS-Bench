include ../generator/Makefile
CC=gcc
PGMS=bandwidth latency
BASE_SRC=$(wildcard ../generator/*.c)
BASE_O=$(patsubst %.c,%.o,$(BASE_SRC))
override CFLAGS+=-O2 -Wall -g -I../generator -DGCC -DEXTENDED_REPORT
CXXFLAGS=$(CFLAGS)
override LDFLAGS+=-Wl,--no-as-needed

all: $(PGMS)

latency: latency.o $(BASE_O)
	$(CC) $(CFLAGS) $< $(BASE_SRC) -o $@ $(LDFLAGS)

bandwidth: bandwidth.o $(BASE_O)
	$(CC) $(CFLAGS) $< $(BASE_SRC) -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(PGMS) $(BASE_O)
