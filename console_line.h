#ifndef WEBCLI_CONSOLE_LINE_H_
#define WEBCLI_CONSOLE_LINE_H_

#include "shared.h"

class ConsoleLine {
public:
	ConsoleLine(const char *line);
	ConsoleLine(const std::string &line);
	virtual ~ConsoleLine();

	const std::string get_time();
	const std::string get_line();

	int compare(ConsoleLine &cl);

	bool is_before(ConsoleLine &cl);
	bool is_after(ConsoleLine &cl);

	bool is_before_time(std::string &time);
	bool is_after_time(std::string &time);

private:
	void initialize(const std::string &line);

	std::string *line_;
	std::string *time_;
};

typedef std::shared_ptr<ConsoleLine> ConsoleLinePtr;
typedef std::deque<ConsoleLinePtr> ConsoleLinePtrDeque;
typedef std::shared_ptr<ConsoleLinePtrDeque> ConsoleLinePtrDequePtr;

#endif /* WEBCLI_CONSOLE_LINE_H_ */
