CC=gcc
PGMS=overhead
BASE_SRC=$(wildcard ../base/*.c)
BASE_O=$(patsubst %.c,%.o,$(BASE_SRC))
CFLAGS=-O2 -Wall -g -I../base -DGCC
CXXFLAGS=$(CFLAGS)
LDFLAGS=-lrt -lm -pthread -Wl,--wrap=malloc -Wl,--wrap=mmap


ifdef CORE
ifeq ($(CORE),CORTEX_A53)
override CFLAGS +=-DAARCH64 -DCORTEX_A53
endif
endif


all: $(PGMS)
overhead:overhead.o $(BASE_O)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(BASE_SRC) -o $@
clean:
	rm *.o $(PGMS) $(BASE_O)
