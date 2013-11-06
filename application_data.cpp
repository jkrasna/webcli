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
	if(argument_list_) {
		delete argument_list_;
	}
	delete run_path_;
	delete application_;
	delete arguments_;
	delete mutex_;
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

const char **ApplicationData::getArgumentList() {
	std::lock_guard<std::mutex> _(*mutex_);

	// Only create argument list if it was changed
	// or if it was not created before
	if(arguments_changed_ || !argument_list_) {
		// First free argument list
		delete argument_list_;

		// Get size of the arguments vector
		int size = arguments_->size();

		// Allocate memory for the argument pointer list
		argument_list_ = new const char *[size + 2];

		// The first parameter must be the program name
		argument_list_[0] = application_->c_str();

		std::string *argument;
		for(int i=0; i < size; i++) {
			argument = arguments_->at(i).get();
			argument_list_[i+1] = argument->c_str();
		}

		// The pointer list must be NULL-terminated
		argument_list_[size+1] = NULL;
	}

	return argument_list_;
}
