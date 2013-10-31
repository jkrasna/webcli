#include "application_console_line.h"

ApplicationConsoleLine::ApplicationConsoleLine(unsigned int index, const char *line) {
	index_ = index;
	line_ = new std::string(line);
}

ApplicationConsoleLine::ApplicationConsoleLine(unsigned int index, const std::string line) {
	index_ = index;
	line_ = new std::string(line);
}

ApplicationConsoleLine::~ApplicationConsoleLine() {
	delete line_;
}

int ApplicationConsoleLine::getIndex() {
	return index_;
}

std::string ApplicationConsoleLine::getLine() {
	return *line_;
}

