.Phony: all clean

PROG_NAME=ppc_viewer
CC=gcc
CFLAGS=-Wall -g -O2

OBJS=ppc_viewer.o util.o main.o
all: $(OBJS) 
	$(CC) $(CFLAGS) -o $(PROG_NAME) $(OBJS) 

main.o: main.c ppc_viewer.h util.h
ppc_viewer.o: ppc_viewer.c ppc_viewer.h util.h
util.o: util.c ppc_viewer.h util.h

clean:
	rm -rf $(PROG_NAME) *.o 
