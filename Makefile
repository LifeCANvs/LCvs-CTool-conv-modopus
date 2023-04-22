SHELL = /bin/sh
SRCDIR = src
TMPDIR = build
BINDIR = bin

EXECNAME = modopus 
MAIN = $(BINDIR)/$(EXECNAME)
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(TMPDIR)/%.o)

CC = clang 
CFLAGS = -Wall -Werror -Wextra -pedantic -g -I /usr/include/opus
LIBS = $(shell pkg-config --libs libopenmpt libopusenc)

.PHONY: all clean build bin

all: $(MAIN)

$(MAIN): $(OBJECTS)
	mkdir -p $(BINDIR)
	$(CC) $(LIBS) $^ -o $@
	
$(TMPDIR)/%.o : $(SRCDIR)/%.c build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p $(TMPDIR)

bin:
	mkdir -p $(BINDIR)

clean:
	rm -f $(OBJECTS) $(MAIN)
