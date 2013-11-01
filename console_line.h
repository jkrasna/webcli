#ifndef WEBCLI_APPLICATION_CONSOLE_LINE_H_
#define WEBCLI_APPLICATION_CONSOLE_LINE_H_

#include "precompiled.h"

class ConsoleLine {
public:
	ConsoleLine(unsigned long index, const char *line);
	ConsoleLine(unsigned long index, const std::string line);
	virtual ~ConsoleLine();

	int getIndex();
	char *getLine();

private:
	void initialize(unsigned long index, const char *line);

	unsigned long index_;
	std::shared_ptr<char> line_;
};

typedef std::shared_ptr<ConsoleLine> consoleLinePtr;

#endif /* WEBCLI_APPLICATION_CONSOLE_LINE_H_ */
