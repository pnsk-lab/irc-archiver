# $Id$

PWD = `pwd`
PLATFORM = generic

FLAG = PWD=$(PWD) PLATFORM=$(PLATFORM)

.PHONY: all clean ./Bot

all: ./Bot

./Bot::
	$(MAKE) -C $@ $(FLAGS)

clean:
	$(MAKE) -C $@ $(FLAGS) clean
