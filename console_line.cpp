#include "console_line.h"

#include <string.h>

ConsoleLine::ConsoleLine(unsigned long index, const char *line) {
	initialize(index, line);
}

ConsoleLine::ConsoleLine(unsigned long index, const std::string line) {
	initialize(index, line.c_str());
}

void ConsoleLine::initialize(unsigned long index,const char *line) {
	index_ = index;
	size_ = strlen(line);
	char *new_line = (char *)calloc(size_ + 1, sizeof(char));
	strcpy(new_line, line);

	line_.reset(new_line);
}

ConsoleLine::~ConsoleLine() {
	line_.reset();
}

unsigned long ConsoleLine::getIndex() {
	return index_;
}

char *ConsoleLine::getLine() {
	return line_.get();
}

unsigned long ConsoleLine::getSize() {
	return size_;
}
