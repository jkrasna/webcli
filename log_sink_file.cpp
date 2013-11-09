#include "log_sink_file.h"

#include <stdarg.h>
#include <ctime>

#define TIME_BUFFER_LENGTH		40

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

	int written = 0;
	char time_string[TIME_BUFFER_LENGTH];
	time_t now = time(NULL);
	tm *now_local = localtime(&now);
	written = strftime(time_string, TIME_BUFFER_LENGTH, "%Y-%m-%d %H:%M:%S ", now_local);

	log_file_->write(time_string, written);
	log_file_->write(message.c_str(), message.size());
	log_file_->flush();
}
