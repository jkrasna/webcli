CC=g++
CCFLAGS=-Wall -g -std=c++11

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

LIBS=-lfcgi++ -lfcgi -lboost_thread -lboost_system

MAIN=webcli

all: $(MAIN)

precompiled.cpp: precompiled.h
%.cpp: precompiled.h
application_data.cpp: application_data.h
application_interface.cpp: application_interface.h application_data.h
web_interface.cpp: web_interface.h application_interface.h
main.cpp: main.h web_interface.h application_interface.h


%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@
	
$(MAIN): $(OBJECTS)
	$(LINK.cpp) $^ -o $@ $(LIBS) 

clean:
	rm -f $(OBJECTS) $(MAIN)

