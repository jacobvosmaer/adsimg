CFLAGS += -std=gnu89 -pedantic -Wall -Werror
OBJS = toc adsimg.o split

all: toc split

toc: adsimg.o
split: adsimg.o

.PHONY: clean
clean:
	rm -f -- $(OBJS)
