CC = gcc
CFLAGS = -g
TARGET = libs.o
.DEFAULT_GOAL = $(TARGET)
DESTDIR = objs
OBJLST = muze_stdlib.o libunwind.a
OBJS = $(foreach obj, $(OBJLST), $(DESTDIR)/$(obj))

$(DESTDIR):
	mkdir -p $(DESTDIR)

$(DESTDIR)/muze_stdlib.o: $(DESTDIR)
	cd .. && make build/morph_graph.o
	cp ../build/morph_graph.o $(DESTDIR)
	cd libmuzeruntime && make
	cp libmuzeruntime/muze_stdlib.o $(DESTDIR)

$(DESTDIR)/libunwind.a: $(DESTDIR)
	cd libunwind && ./autogen.sh && ./configure CFLAGS="-g" && mkdir -p 'usr/local' && make && make install prefix=$(shell pwd)/libunwind/usr/local
	cp libunwind/usr/local/lib/libunwind.a $(DESTDIR)

$(TARGET): $(OBJS)
	ld -r -o $(TARGET) $(shell ls -d $(DESTDIR)/*)

.PHONY = clean

clean:
	rm -rf $(DESTDIR) $(TARGET)
	cd libmuzeruntime && make clean
	cd libunwind && make clean && rm -rf usr
