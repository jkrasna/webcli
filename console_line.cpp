#include "console_line.h"

#include "utils.h"

ConsoleLine::ConsoleLine(const char *line) {
	std::string line_string(line);
	initialize(line_string);
}

ConsoleLine::ConsoleLine(const std::string &line) {
	initialize(line);
}

void ConsoleLine::initialize(const std::string &line) {
	time_ = new std::string(Utils::get_iso_time());
	line_ = new std::string(line);
}

ConsoleLine::~ConsoleLine() {
	delete line_;
	delete time_;
}

const std::string ConsoleLine::get_line() {
	return *line_;
}

const std::string ConsoleLine::get_time() {
	return *time_;
}

int ConsoleLine::compare(ConsoleLine &cl) {
	return (*time_).compare(cl.get_time());
}

bool ConsoleLine::is_before(ConsoleLine &cl) {
	return compare(cl) < 0;
}

bool ConsoleLine::is_after(ConsoleLine &cl) {
	return compare(cl) > 0;
}

bool ConsoleLine::is_before_time(std::string &time) {
	return (*time_).compare(time) < 0;
}

bool ConsoleLine::is_after_time(std::string &time) {
	return (*time_).compare(time) > 0;
}
