CC=gcc
CFLAGS=-I.
DEPS = htmlParser.h

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

httpServer: httpServer.o htmlParser.o
	$(CC) -g -o httpServer httpServer.o htmlParser.o

.PHONY: clean

clean:
	rm -f *.o