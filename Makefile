CFLAGS += -std=gnu89 -pedantic -Wall -Werror
OBJS = toc adsimg.o split err.o

all: toc split

toc: adsimg.o err.o
split: adsimg.o err.o

.PHONY: clean
clean:
	rm -f -- $(OBJS)
