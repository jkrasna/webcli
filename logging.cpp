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
		case LL_NO: 		return "[NONE]:    ";
		case LL_CRITICAL: 	return "[CRITICAL]:";
		case LL_ERROR: 		return "[ERROR]:   ";
		case LL_WARNING: 	return "[WARNING]: ";
		case LL_DEBUG: 		return "[DEBUG]:   ";
		case LL_TRACE: 		return "[TRACE]:   ";
		case LL_ALL: 		return "[ALL]:     ";
		default: 			return "[unknown]: ";
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


