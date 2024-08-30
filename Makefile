# $Id$

PWD = `pwd`
PLATFORM = generic

FLAGS = PWD=$(PWD) PLATFORM=$(PLATFORM)

.PHONY: all format clean ./Bot ./Web

all: ./Bot

./Bot:: ./Web
	$(MAKE) -C $@ $(FLAGS)

./Web::
	$(MAKE) -C $@ $(FLAGS)

format:
	clang-format --verbose -i `find . -name "*.c" -or -name "*.h"`

clean:
	$(MAKE) -C ./Bot $(FLAGS) clean
	$(MAKE) -C ./Web $(FLAGS) clean
