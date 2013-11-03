#ifndef WEBCLI_LOGGING_H_
#define WEBCLI_LOGGING_H_

#include <iostream>
#include <fstream>
#include <string>

enum LogLevel {
	LOG_LEVEL_SYSTEM,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_VERBOSE
};

class Logging {
public:
	Logging(std::string fileName);
	virtual ~Logging(void);

	void set_log_level(int log_level);
	bool is_open(void);

	static void log_line(int log_level, char *line);

private:
	int log_level_;
	std::ofstream *log_file_;
	bool open_;
};

#endif /* WEBCLI_LOGGING_H_ */
