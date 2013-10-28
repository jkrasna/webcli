#include "application_data.h"

ApplicationData::ApplicationData(std::string run_path, std::string application) {
	run_path_ = new std::string(run_path);
	application_ = new std::string(application);
	application_path_ = NULL;
	arguments_ = new std::vector<std::string>();
	mutex_ = new std::mutex();
}

ApplicationData::ApplicationData(std::string run_path, std::string application, std::string application_path) {
	run_path_ = new std::string(run_path);
	application_ = new std::string(application);
	application_path_ = new std::string(application_path);
	arguments_ = new std::vector<std::string>();
	mutex_ = new std::mutex();
}

ApplicationData::~ApplicationData() {
	delete run_path_;
	delete application_;
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

std::string ApplicationData::getRunPath() {
	std::lock_guard<std::mutex> _(*mutex_);
	return *run_path_;
}

std::string ApplicationData::getApplication() {
	std::lock_guard<std::mutex> _(*mutex_);
	return *application_;
}

std::string ApplicationData::getApplicationPath() {
	std::lock_guard<std::mutex> _(*mutex_);
	return *application_path_;
}

std::string ApplicationData::getSpaceSeparatedArgumentList() {
	std::lock_guard<std::mutex> _(*mutex_);
	std::string list("");

	//TODO: Create space separated argument list

	return list;
}
