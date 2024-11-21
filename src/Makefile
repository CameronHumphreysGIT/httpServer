CC=/usr/bin/gcc
CFLAGS=-I$(IDIR)
IDIR =../include
ODIR =obj

_DEPS = htmlParser.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
_OBJ = httpServer.o htmlParser.o
OBJ =  $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -g -c -o $@ $< $(CFLAGS)

httpServer: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o httpServer