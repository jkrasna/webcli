#ifndef WEBCLI_LOG_SINK_FILE_H_
#define WEBCLI_LOG_SINK_FILE_H_

#include "shared.h"

#include "log_sink.h"

class LogSinkFile : public LogSink {
public:
	LogSinkFile(int level, const std::string filename);
	virtual ~LogSinkFile();

	virtual void log(int level, std::string);

private:
	std::ofstream *log_file_;
};

#endif /* WEBCLI_LOG_SINK_FILE_H_ */
