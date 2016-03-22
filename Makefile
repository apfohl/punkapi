BIN = punkapi

VERSION=$(shell git describe --tags --dirty=+ || echo "UNKNOWN")

CFLAGS = -g -Wall -std=gnu11 -DVERSION=\"$(VERSION)\"
LDFLAGS =
LDLIBS = -lcurl -ljzon

.PHONY: all release style clean

all: $(BIN)

release: CFLAGS = -std=gnu11 -Os -march=native -flto -Wall -Wextra -Wpedantic -Wstrict-overflow -fno-strict-aliasing -DVERSION=\"$(VERSION)\"
release: $(BIN)

style:
	astyle -A3s4SpHk3jn punkapi.c

clean:
	rm -f $(BIN)
	rm -rf *.dSYM
