CC=g++
CCFLAGS=-Wall -g -std=c++11

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

LIBS=-lfcgi++ -lfcgi -lboost_thread -lboost_system -lutil

MAIN=webcli

all: $(MAIN)

application_data.cpp: 			shared.h application_data.h 
application_interface.cpp: 		shared.h application_interface.h application_data.h
web_interface.cpp: 				shared.h web_interface.h application_interface.h
main.cpp: 						shared.h main.h web_interface.h application_interface.h


%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@
	
$(MAIN): $(OBJECTS)
	$(LINK.cpp) $^ -o $@ $(LIBS) 

clean:
	rm -f $(OBJECTS) $(MAIN)

