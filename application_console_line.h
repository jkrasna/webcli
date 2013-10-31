#ifndef WEBCLI_APPLICATION_CONSOLE_LINE_H_
#define WEBCLI_APPLICATION_CONSOLE_LINE_H_

#include "precompiled.h"

class ApplicationConsoleLine {
public:
	ApplicationConsoleLine(unsigned int index, const char *line);
	ApplicationConsoleLine(unsigned int index, const std::string line);
	virtual ~ApplicationConsoleLine();

	int getIndex();
	std::string getLine();

private:
	int index_;
	std::string *line_;
};

typedef std::shared_ptr<ApplicationConsoleLine> ApplicationConsoleLinePtr;

#endif /* WEBCLI_APPLICATION_CONSOLE_LINE_H_ */
