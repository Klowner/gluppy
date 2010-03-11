OBJS = src/latitude.o src/gluppyd.o
PROG = gluppyd
PROG_SO = libgluppy.so
CFLAGS += `pkg-config --cflags glib-2.0 gtk+-2.0 gconf-2.0 libcurl liblocation hildon-control-panel libosso` -Os -Wall
LDFLAGS += `pkg-config --libs glib-2.0 gtk+-2.0 gconf-2.0 libcurl liblocation hildon-control-panel libosso`
HILDON_DESKTOP_DIR = `pkg-config hildon-control-panel --variable=plugindesktopentrydir`
HILDON_LIB_DIR = `pkg-config hildon-control-panel --variable=pluginlibdir`

DESTDIR ?=
PREFIX ?= /usr

all: $(PROG) $(PROG_SO)

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@
	strip $@

$(PROG_SO): src/libgluppy.c
	$(CC) -shared $(CFLAGS) $(LDFLAGS) $^ -o $@

#src/libgluppy.so: src/libgluppy.c
src/latitude.o: src/latitude.c src/latitude.h
src/gluppyd.o: src/gluppyd.c src/latitude.o src/gconf-keys.h

install: $(PROG)
	mkdir -p $(DESTDIR)$(PREFIX)/sbin/
	install $(PROG) $(DESTDIR)$(PREFIX)/sbin/$(PROG)
	mkdir -p $(DESTDIR)/etc/event.d/
	install upstart/$(PROG) $(DESTDIR)/etc/event.d/
	mkdir -p $(DESTDIR)$(HILDON_LIB_DIR)
	install $(PROG_SO) $(DESTDIR)$(HILDON_LIB_DIR)/$(PROG_SO)
	mkdir -p $(DESTDIR)$(HILDON_DESKTOP_DIR)
	install gluppy.desktop $(DESTDIR)$(HILDON_DESKTOP_DIR)/gluppy.desktop

clean:
	rm -f $(PROG) $(PROG_SO) $(OBJS)

.DEFAULT: all
