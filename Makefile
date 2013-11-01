CC=g++
CCFLAGS=-Wall -g -std=c++11

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

LIBS=-lfcgi++ -lfcgi -lboost_thread -lboost_system -lutil

MAIN=webcli

all: $(MAIN)

precompiled.cpp: 				precompiled.h
application_data.cpp: 			precompiled.h application_data.h 
application_interface.cpp: 		precompiled.h application_interface.h application_data.h
web_interface.cpp: 				precompiled.h web_interface.h application_interface.h
main.cpp: 						precompiled.h main.h web_interface.h application_interface.h


%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@
	
$(MAIN): $(OBJECTS)
	$(LINK.cpp) $^ -o $@ $(LIBS) 

clean:
	rm -f $(OBJECTS) $(MAIN)

