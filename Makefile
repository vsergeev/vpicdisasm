CC = gcc
CFLAGS = -Wall -O3 -D_GNU_SOURCE
LDFLAGS=
OBJECTS = libGIS-1.0.4/ihex.o libGIS-1.0.4/srecord.o pic_instructionset.o pic_disasm.o format.o file.o ui.o
PROGNAME = vpicdisasm
BINDIR = /usr/local/bin

all: $(PROGNAME)

install: $(PROGNAME)
	install -m 0755 $(PROGNAME) $(BINDIR)

$(PROGNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $(OBJECTS) 

clean:
	rm -rf $(PROGNAME) libGIS-1.0.4/*.o *.o

uninstall:
	rm -f $(BINDIR)/$(PROGNAME)
