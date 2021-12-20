CC=gcc
CXX=g++
PGMS=latency bandwidth
BASE_DIR=../base
BASE_SRC := $(wildcard $(BASE_DIR)/*.c)
BASE_O := $(wildcard $(BASE_DIR)/*.o)
CFLAGS=-O2 -Wall -g -I$(BASE_DIR)
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Wl,--no-as-needed -lrt

all:base $(PGMS)
latency:base latency.o
bandwidth:base bandwidth.o
base: $(BASE_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@
clean:
	rm *.o *~ $(PGMS) $(BASE_O)
