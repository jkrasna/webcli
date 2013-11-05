#ifndef WEBCLI_LOG_SINK_H_
#define WEBCLI_LOG_SINK_H_

#include "shared.h"

class LogSink {
public:
	LogSink(int level);
	virtual ~LogSink();

	virtual void log(int level, std::string message);

	int get_level();
	void set_level(int level);

protected:
	int level_;
	std::mutex *mutex_;
};

#endif /* WEBCLI_LOG_SINK_H_ */
