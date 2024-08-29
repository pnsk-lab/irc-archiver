# $Id$

PWD = `pwd`
PLATFORM = generic

FLAGS = PWD=$(PWD) PLATFORM=$(PLATFORM)

.PHONY: all format clean ./Bot

all: ./Bot

./Bot::
	$(MAKE) -C $@ $(FLAGS)

format:
	clang-format --verbose -i `find . -name "*.c" -or -name "*.h"`

clean:
	$(MAKE) -C $@ $(FLAGS) clean
