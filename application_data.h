#ifndef WEBCLI_APPLICATION_DATA_H_
#define WEBCLI_APPLICATION_DATA_H_

#include "precompiled.h"

typedef std::shared_ptr<std::string> stringPtr;

class ApplicationData {
public:
	ApplicationData(std::string run_path, std::string application, bool search=false);
	virtual ~ApplicationData();

	void addNewArgument(char *argument);
	void addNewArgument(std::string argument);

	const char *getRunPath();
	const char *getApplication();
	bool isSearchEnabled();
	char **getArgumentList();

private:
	void initialize(std::string run_path, std::string application, bool search);
	void freeArgumentList();

	std::string *run_path_;
	std::string *application_;
	bool search_;

	std::vector<stringPtr> *arguments_;
	std::mutex *mutex_;

	char **argument_list_;
	bool arguments_changed_;
};

#endif /* WEBCLI_APPLICATION_DATA_H_ */
