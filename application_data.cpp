#include "application_data.h"

#include <string.h>

ApplicationData::ApplicationData(std::string run_path, std::string application, bool search) {
	initialize(run_path, application, search);
}

void ApplicationData::initialize(std::string run_path, std::string application, bool search) {
	run_path_ 		= new std::string(run_path);
	application_ 	= new std::string(application);
	search_ 		= search;

	arguments_		= new std::vector<stringPtr>();
	mutex_			= new std::mutex();

	argument_list_		= NULL;
	arguments_changed_	= true;
}

ApplicationData::~ApplicationData() {
	freeArgumentList();
	delete run_path_;
	delete application_;
	delete arguments_;
	delete mutex_;
}

void ApplicationData::freeArgumentList() {
	if(!argument_list_) {
		return;
	}

	for(int i=0; argument_list_[i]; i++) {
		free(argument_list_[i]);
	}

	free(argument_list_);
	argument_list_ = NULL;
}

void ApplicationData::addNewArgument(char *argument) {
	std::lock_guard<std::mutex> _(*mutex_);
	arguments_->push_back(stringPtr(new std::string(argument)));
	arguments_changed_ = true;
}

void ApplicationData::addNewArgument(std::string argument) {
	std::lock_guard<std::mutex> _(*mutex_);
	arguments_->push_back(stringPtr(new std::string(argument)));
	arguments_changed_ = true;
}

const char *ApplicationData::getRunPath() {
	return run_path_->c_str();
}

const char *ApplicationData::getApplication() {
	return application_->c_str();
}

bool ApplicationData::isSearchEnabled() {
	return search_;
}

char **ApplicationData::getArgumentList() {
	std::lock_guard<std::mutex> _(*mutex_);

	int size;
	std::string *argument;

	if(arguments_changed_ || !argument_list_) {
		freeArgumentList();

		size = arguments_->size();

		argument_list_ = (char **)calloc(size + 1, sizeof(char *));

		for(int i=0; i< size; i++) {
			argument = (*arguments_)[i].get();

			argument_list_[i] = (char *)calloc(argument->size() + 1, sizeof(char));
			strncpy(argument_list_[i], argument->c_str(), argument->size());
		}
	}

	return argument_list_;
}
