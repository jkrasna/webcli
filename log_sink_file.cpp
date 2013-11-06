#include "log_sink_file.h"

#include <stdarg.h>

#define STRING_BUFFER		1024

LogSinkFile::LogSinkFile(int level, const std::string filename) : LogSink(level) {
	log_file_ = (std::ofstream *)new std::ofstream(filename);
}

LogSinkFile::~LogSinkFile() {
	if (log_file_) {
		log_file_->close();
		delete log_file_;
	}
}

void LogSinkFile::log(int level, std::string message) {
	std::lock_guard<std::mutex> _(*mutex_);

	if(message.back() != '\n') {
		message.append("\n");
	}

	log_file_->write(message.c_str(), message.size());
	log_file_->flush();
}
