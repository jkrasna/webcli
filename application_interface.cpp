#include "application_interface.h"

#include <unistd.h>

ApplicationInterface::ApplicationInterface(
		std::shared_ptr<ApplicationData> application_data) {
	initialize(application_data);
}

void ApplicationInterface::initialize(
		std::shared_ptr<ApplicationData> application_data) {
	application_data_ = application_data;
	application_input_messages_ = new std::deque<ApplicationConsoleLinePtr>();
	application_output_messages_ = new std::deque<ApplicationConsoleLinePtr>();

	worker_thread_ = new std::thread(&ApplicationInterface::worker, this);
}

ApplicationInterface::~ApplicationInterface() {
	if (worker_thread_) {
		worker_thread_->join();
		delete worker_thread_;
	}

	delete application_input_messages_;
	delete application_output_messages_;

	application_data_.reset();
}

int ApplicationInterface::start_subprocess() {
	// TODO: Initialize and start application (check application path, fork, set session, create process group, execvp,...)

	child_pid_ = fork();

	if (child_pid_ < 0) {
		//TODO: Fork failed - log message
		return applicationError;
	} else if (child_pid_ == 0) {
		// Child logic
		int rc = chdir(application_data_->getRunPath());
		if(rc < 0) {
			// TODO: Error - unable to change directory to specified dir
		}

		if(application_data_->isSearchEnabled()) {
			execvp(application_data_->getApplication(), application_data_->getArgumentList());
		}
		else {
			execv(application_data_->getApplication(), application_data_->getArgumentList());
		}
	} else {
		//TODO: Parent logic
	}

	return applicationSuccess;
}

void ApplicationInterface::worker() {
	bool running = true;

	if (!this->start_subprocess()) {
		//TODO: throw initialization error
	}

	while (running) {
		//TODO: read messages from terminal

		//TODO: Add messages to terminal
	}
}
