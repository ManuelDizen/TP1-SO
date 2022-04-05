CC = gcc
CFLAGS = -Wall -g -pthread -std=gnu99
TARGETS = view solve slave
SOURCES = queue

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(CFLAGS) $(SOURCES:=.c) $@.c -lrt -o $@
	@rm -f $(TARGETS:=.o)

clean:
	@rm -f $(TARGETS) $(TARGETS:=.o) 
	@rm -f $(SOURCES) $(SOURCES:=.o)