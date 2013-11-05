#include "log_sink.h"

#include <stdarg.h>

LogSink::LogSink(int level) {
	level_ = level;
	mutex_ = new std::mutex();
}

LogSink::~LogSink() {
	delete mutex_;
}

int LogSink::get_level() {
	std::lock_guard<std::mutex> _(*mutex_);
	return level_;
}

void LogSink::set_level(int level) {
	std::lock_guard<std::mutex> _(*mutex_);
	level_ = level;
}

void LogSink::log(int level, std::string message) {
	std::lock_guard<std::mutex> _(*mutex_);

	if(message.back() != '\n') {
		message.append("\n");
	}

	printf("%s", message.c_str());
}
