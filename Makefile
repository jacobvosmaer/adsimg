CFLAGS += -std=gnu89 -pedantic -Wall -Werror
OBJS = toc adsimg.o

all: toc

toc: adsimg.o

.PHONY: clean
clean:
	rm -f -- $(OBJS)
