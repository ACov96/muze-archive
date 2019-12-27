BUILDDIR = build
SRCDIR = src
INCLUDEDIR = include

CC = gcc
CFLAGS = -g -Wall -Wfatal-errors -Werror \
	 -Wno-error=unused-function \
	 -Wno-error=unused-variable \
	 -Wno-error=unused-parameter \
	 -Wno-error=unused-value \
	 -Wno-error=unused-label \
	 -I $(INCLUDEDIR) \
	 -I ../../include \
	 -lm \
	 -no-pie

OBJLST = muze_stdlib.o
OBJS = $(foreach obj, $(OBJLST), $(BUILDDIR)/$(obj))

TARGET = muze_stdlib.o

vpath %.c $(SRCDIR)
vpath %.h $(INCLUDEDIR)

$(BUILDDIR) :
	mkdir -p $@

$(BUILDDIR)/%.o : %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET) : $(OBJS)
	cp $(BUILDDIR)/$(TARGET) .

.DEFAULT_GOAL = $(TARGET)

.PHONY : clean test

TESTSCRIPT = test.sh

test: $(TARGET)
	./$(TESTSCRIPT)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f a.s
	rm -rf $(BUILDDIR)
