CC = gcc
CFLAGS = -Wall -g -pthread -std=gnu99
TARGETS = view solve slave
SOURCES = queue
INSTALL = apt-get install
MINISAT = minisat

all: dependencies clean $(TARGETS)

dependencies:
	$(INSTALL) $(MINISAT)

$(TARGETS):
	$(CC) $(CFLAGS) $(SOURCES:=.c) $@.c -lrt -o $@
	@rm -f $(TARGETS:=.o)

clean:
	@rm -f $(TARGETS) $(TARGETS:=.o) 
	@rm -f $(SOURCES) $(SOURCES:=.o)