include config.mk

SRC = hotman.c
DEPS = config.h types.h
OBJ = $(SRC:.c=.o)

all: hotman

.c.o:
	$(CC) $(STCFLAGS) -c $<

hotman.o: config.h

$(OBJ): config.h config.mk

hotman: $(OBJ)
	$(CC) -o $@ $(OBJ) $(STCFLAGS) -lxcb -lxcb-keysyms

clean:
	rm -f hotman $(OBJ)

install: hotman
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f hotman $(DESTDIR)$(PREFIX)/bin
	chmod 755 $(DESTDIR)$(PREFIX)/bin/hotman

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/hotman
