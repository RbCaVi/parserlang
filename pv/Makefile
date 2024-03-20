flags := -Wall -Wpedantic -pedantic-errors -Wextra
compileflags :=
linkflags :=
arflags := crv

compilecmd = $(CC) $(flags) $(compileflags) $(CFLAGS) -c
linkcmd = $(CC) $(flags) $(linkflags) $(LDFLAGS)
archivecmd = $(AR) $(arflags) $(ARFLAGS)

.PHONY: all clean

all: pv.so pv.a

%.o: %.c
	$(compilecmd) $< -o $@

pv.o: pv_alloc.h pv.h pv_unicode.h util.h
pv_alloc.o: pv_alloc.h pv.h
pv_aux.o: pv_alloc.h pv_private.h pv.h
pv_unicode.o: pv_unicode.h pv_utf8_tables.h
util.o: util.h

pv.so: pv.o pv_alloc.o pv_aux.o pv_unicode.o util.o
	$(linkcmd) $< -o $@ -shared

pv.a: pv.a(pv.o pv_alloc.o pv_aux.o pv_unicode.o util.o)
	$(archivecmd) $@ $?