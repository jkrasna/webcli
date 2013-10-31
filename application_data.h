#ifndef WEBCLI_APPLICATION_DATA_H_
#define WEBCLI_APPLICATION_DATA_H_

#include "precompiled.h"

class ApplicationData {
public:
	ApplicationData(std::string run_path, std::string application);
	ApplicationData(std::string run_path, std::string application, std::string application_path);
	virtual ~ApplicationData();

	void addNewArgument(std::string argument);

	std::string getRunPath();
	std::string getApplication();
	std::string getApplicationPath();
	bool isApplicationPathSet();
	char **getArgumentList();

private:
	void initialize(std::string run_path, std::string application, std::string application_path);

	std::string *run_path_;
	std::string *application_;
	std::string *application_path_;
	bool application_path_set_;

	std::vector<std::string> *arguments_;
	std::mutex *mutex_;
};

#endif /* WEBCLI_APPLICATION_DATA_H_ */
