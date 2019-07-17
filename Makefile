SRC = hotman.c
OBJ = $(SRC:.c=.o)

PREFIX = /usr/local

all: hotman

.c.o:
	$(CC) -I/usr/X11R6/include -c  $<

hotman: $(OBJ)
	$(CC) -g -o $@ $(OBJ) -I/usr/X11R6/include -L/usr/X11R6/lib -lxcb -lxcb-keysyms

install: hotman
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f hotman $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/hotman

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/hotman $(OBJ)

clean:
	rm -f hotman $(OBJ)
