SHELL = /bin/sh
SRCDIR = src

EXECNAME = modopus 
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:%.c=%.o)

CC = clang 
CFLAGS = -Wall -Werror -Wextra -pedantic -g -I /usr/include/opus
LIBS = -lopenmpt -lopus -lopusenc

.PHONY: all clean

all: $(EXECNAME)

$(EXECNAME): $(OBJECTS)
	$(CC) $(LIBS) $^ -o $@
	
%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm  -f $(EXECNAME) $(SRCDIR)/*.o
