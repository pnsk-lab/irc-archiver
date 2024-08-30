# $Id$

include $(PWD)/Platform/$(PLATFORM).mk

.PHONY: all clean
.SUFFIXES: .c .o

OBJS = main.o util.o db.o bot.o ircfw.o ../Web/web.a

all: ircarc$(EXEC)

ircarc$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -I $(PWD)/Web -c -o $@ $<

clean:
	rm -f *.o *.exe ircarc
