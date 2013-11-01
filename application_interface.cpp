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
	pid_t pid = 0;

	pid = fork();

	if (pid < 0) {
		//TODO: Fork failed - log message
		return applicationError;
	} else if (pid == 0) {
		//TODO: Child logic
		if(application_data_->isApplicationPathSet()) {
			execve(application_data_->getApplicationName().c_str(), application_data_->getArgumentList(), NULL);
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
