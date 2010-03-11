OBJS = src/latitude.o src/gluppyd.o
PROG = gluppyd
CFLAGS += `pkg-config --cflags gtk+-2.0 gconf-2.0 libcurl liblocation` -Os -Wall
LDFLAGS += `pkg-config --libs gtk+-2.0 gconf-2.0 libcurl liblocation`

DESTDIR ?=
PREFIX ?= /usr

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@
	strip $@

src/latitude.o: src/latitude.c src/latitude.h
src/gluppyd.o: src/gluppyd.c src/latitude.o src/gconf-keys.h

install: $(PROG)
	install $(PROG) $(DESTDIR)$(PREFIX)/bin/$(PROG)
	install $(PROG).launch $(DESTDIR)$(PREFIX)/bin/$(PROG).launch
	mkdir -p $(DESTDIR)/etc/event.d/
	install upstart/$(PROG) $(DESTDIR)/etc/event.d/

clean:
	rm -f $(PROG) $(OBJS)

.DEFAULT: all
