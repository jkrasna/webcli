#include "application_data.h"

#include <string.h>

ApplicationData::ApplicationData(std::string run_path, std::string application) {
	run_path_ = new std::string(run_path);
	application_name_ = new std::string(application);
	application_path_ = NULL;
	application_path_set_ = false;
	arguments_ = new std::vector<std::string>();
	mutex_ = new std::mutex();
}

ApplicationData::ApplicationData(std::string run_path, std::string application, std::string application_path) {
	run_path_ = new std::string(run_path);
	application_name_ = new std::string(application);
	application_path_ = new std::string(application_path);
	application_path_set_ = true;
	arguments_ = new std::vector<std::string>();
	mutex_ = new std::mutex();
}

ApplicationData::~ApplicationData() {
	delete run_path_;
	delete application_name_;
	if(application_path_) {
		delete application_path_;
	}
	delete arguments_;
	delete mutex_;
}

void ApplicationData::addNewArgument(std::string argument) {
	std::lock_guard<std::mutex> _(*mutex_);
	arguments_->push_back(argument);
}

void ApplicationData::addNewArgument(char *argument) {
	std::lock_guard<std::mutex> _(*mutex_);
	std::string arg(argument);
	arguments_->push_back(arg);
}

std::string ApplicationData::getRunPath() {
	std::lock_guard<std::mutex> _(*mutex_);
	return *run_path_;
}

std::string ApplicationData::getApplicationName() {
	std::lock_guard<std::mutex> _(*mutex_);
	return *application_name_;
}

std::string ApplicationData::getApplicationPath() {
	std::lock_guard<std::mutex> _(*mutex_);
	return *application_path_;
}

bool ApplicationData::isApplicationPathSet() {
	std::lock_guard<std::mutex> _(*mutex_);
	return application_path_set_;
}

char **ApplicationData::getArgumentList() {
	std::lock_guard<std::mutex> _(*mutex_);

	int size;
	std::string *argument;
	char **argument_list;

	size = arguments_->size();

	argument_list = (char **)calloc(size + 1, sizeof(char *));

	for(int i=0; i< size; i++) {
		argument = &(*arguments_)[i];

		argument_list[i] = (char *)calloc(argument->size() + 1, sizeof(char));
		strncpy(argument_list[i], argument->c_str(), argument->size());
	}

	return argument_list;
}
