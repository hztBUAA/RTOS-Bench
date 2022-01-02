CC=gcc
PGMS=latency bandwidth
BASE_SRC=$(wildcard ../base/*.c)
BASE_O=$(patsubst %.c,%.o,$(BASE_SRC))
CFLAGS=-O2 -Wall -g -I../base -DGCC
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Wl,--no-as-needed -lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap


all: $(PGMS)
latency:latency.o $(BASE_O)
bandwidth: bandwidth.o
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(BASE_SRC) -o $@
clean:
	rm *.o $(PGMS) $(BASE_O)
