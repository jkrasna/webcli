#include "logging.h"

Logging::Logging() {
	sink_list_ = new std::vector<LogSinkPtr>();
	mutex_ = new std::mutex();
}

Logging::~Logging() {
	if(sink_list_) {
		sink_list_->clear();
		delete sink_list_;
	}
	if(mutex_) {
		delete mutex_;
	}
}

const char *Logging::get_level_string(int level) {
	switch(level) {
		case 0: return "[NONE]:    ";
		case 1: return "[CRITICAL]:";
		case 2: return "[ERROR]:   ";
		case 3: return "[WARNING]: ";
		case 4: return "[DEBUG]:   ";
		case 5: return "[TRACE]:   ";
		case 6: return "[ALL]:     ";
		default: return "[unknown]: ";
	}
}

void Logging::add_sink(LogSink *sink) {
	const Logging& instance = Logging::get_instance();
	std::lock_guard<std::mutex> _(*instance.mutex_);
	instance.sink_list_->emplace_back(sink);
}

void Logging::log(int level, std::string message) {
	const Logging& instance = Logging::get_instance();
	std::lock_guard<std::mutex> _(*instance.mutex_);
	for (LogSinkPtr &sink : *instance.sink_list_) {
		if (level <= sink->get_level()) {
			sink->log(level, message);
		}
	}
}


