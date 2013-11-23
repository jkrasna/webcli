CC=g++
CCFLAGS=-Wall -g -std=c++11

SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:.cpp=.o)

LIBS=-lfcgi++ -lfcgi -lboost_regex -lboost_system -lboost_filesystem -lutil

MAIN=webcli

all: $(MAIN)

log_sink.cpp:					shared.h log_sink.h
log_sink_file.cpp:				shared.h log_sink_file.h log_sink.h
logging.cpp:					shared.h logging.h log_sink.h
console_line.cpp:				shared.h logging.h console_line.h
application_data.cpp: 			shared.h logging.h application_data.h 
application_interface.cpp: 		shared.h logging.h application_interface.h application_data.h console_line.h
web_template.cpp:				shared.h logging.h web_template.h
web_interface.cpp: 				shared.h logging.h web_interface.h application_interface.h web_template.h
control_flow.cpp:				shared.h logging.h control_flow.h web_interface.h application_interface.h application_data.h
main.cpp: 						shared.h logging.h main.h control_flow.h log_sink.h log_sink_file.h

%.o: %.cpp
	$(CC) -c $(CCFLAGS) $< -o $@
	
$(MAIN): $(OBJECTS)
	$(LINK.cpp) $^ -o $@ $(LIBS) 

clean:
	rm -f $(OBJECTS) $(MAIN)

