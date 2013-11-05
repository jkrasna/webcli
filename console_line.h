#ifndef WEBCLI_APPLICATION_CONSOLE_LINE_H_
#define WEBCLI_APPLICATION_CONSOLE_LINE_H_

#include "shared.h"

class ConsoleLine {
public:
	ConsoleLine(unsigned long index, const char *line);
	ConsoleLine(unsigned long index, const std::string line);
	virtual ~ConsoleLine();

	unsigned long getIndex();
	char *getLine();
	unsigned long getSize();


private:
	void initialize(unsigned long index, const char *line);

	unsigned long index_;
	std::shared_ptr<char> line_;
	unsigned long size_;
};

typedef std::shared_ptr<ConsoleLine> consoleLinePtr;

#endif /* WEBCLI_APPLICATION_CONSOLE_LINE_H_ */
