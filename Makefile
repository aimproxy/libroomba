CC = clang
LIB_PATH = /usr/local
CFLAGS = -Wall -g -I$(LIB_PATH)/include 
BINS = libroomba.so
SOURCES = src/libroomba.c
HEADER = src/commum.h
BUILDDIR = build
LIBS = -L$(LIB_PATH)/lib -lm

CFLAGS += $(shell pkg-config --cflags json-c)
LDFLAGS += $(shell pkg-config --libs json-c)
LDFLAGS += -lwolfssl

all: dir $(BUILDDIR)/$(BINS)

dir:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/$(BINS): $(SOURCES) $(HEADER)
	$(CC) $(CFLAGS) $(LDFLAGS) -fPIC -shared -o $@ $(SOURCES) -lc $(LIBS)

clean:
	rm -rf $(BUILDDIR)
