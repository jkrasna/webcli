CC=g++
CFLAGS=-Wall -g

SOURCES=main.c web_interface.c application_interface.c
OBJECTS=$(SOURCES:.c=.o)

LIBS=-lfcgi++ -lfcgi -lboost_thread -lboost_system

MAIN=webcli

all: clean $(MAIN)

$(MAIN): $(OBJECTS)
	$(LINK.c) $^ -o $@ $(LIBS)

%.c: %.h 

clean:
	rm -f *.o $(MAIN) *~

