#include "application_interface.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <stdio.h>

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

	child_pid_			= 0;
	child_sid_			= 0;
	fd_terminal_master_	= 0;
	fd_terminal_slave_  = 0;
	slave_name_			= NULL;
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

//! The function that starts the application as a subprocess

//! This function opens the pseudoterminal master and then forks another
//! process with the child pseudoterminal set. It then returns allowing
//! for communication between the terminals.
int ApplicationInterface::start_subprocess() {

	// Open master pseudoterminal
	fd_terminal_master_ = posix_openpt(O_RDWR | O_NOCTTY);

	// Change permissions of the pseudo terminal file and unlock slave side
	if (fd_terminal_master_ < 0 || grantpt (fd_terminal_master_) == -1
	|| unlockpt (fd_terminal_master_) == -1
	|| (slave_name_ = ptsname (fd_terminal_master_)) == NULL) {
		// TODO: Log error here
		return applicationError;
	}

	// TODO: Change into log
	printf("slave device is: %s\n", slave_name_);

	// Open slave terminal - needed for child application
	fd_terminal_slave_ = open(slave_name_, O_RDWR | O_NOCTTY);
	if (fd_terminal_slave_ < 0) {
		// TODO: Log error here
		close(fd_terminal_master_);
		return applicationError;
	}

	// Load pseudo terminal control modules "ptem" and "ldterm" to slave terminal
	if (ioctl(fd_terminal_slave_, I_PUSH, "ptem") == -1
			|| ioctl(fd_terminal_slave_, I_PUSH, "ldterm") == -1) {
		// TODO: Log error here
		close(fd_terminal_master_);
		close(fd_terminal_slave_);
		return applicationError;
	}

	// Creating a new subprocess
	child_pid_ = fork();

	// Delegate functionality depending on the process
	if (child_pid_ < 0) {
		//TODO: Fork failed - log message
		close(fd_terminal_master_);
		close(fd_terminal_slave_);
		return applicationError;

	} else if (child_pid_ == 0) {
		// This is the child process

		// Close master pseudoterminal file descriptor
		close(fd_terminal_master_);

		// Connect slave terminal to input output and error fds
		dup2(fd_terminal_slave_, 0);
		dup2(fd_terminal_slave_, 1);
		dup2(fd_terminal_slave_, 2);

		// Close slave terminal as we do not need it anymore
		close(fd_terminal_slave_);

		// Create new session id for the process group
		if ((child_sid_ = setsid()) == -1) {
			// TODO: Error - unable to create new session
			return applicationError;
		}

		// Change working directory to the configured one
		int rc = chdir(application_data_->getRunPath());
		if (rc < 0) {
			// TODO: Error - unable to change directory to specified dir
			return applicationError;
		}

		// Replace exiting child process with what we want to execute
		if (application_data_->isSearchEnabled()) {
			// If we do not know the path of the application we can let
			// the system search for it in the standard paths
			execvp(application_data_->getApplication(),
					application_data_->getArgumentList());
		} else {
			// This requires that the getApplication method returns the
			// full path of the application
			execv(application_data_->getApplication(),
					application_data_->getArgumentList());
		}

		fprintf(stderr, "ERROR: Unable to start: '%s' - exec failed\n",
				application_data_->getApplication());
		return applicationError;
	} else {
		// This is the parent process

		// Close slave terminal as we do not need it anymore
		close(fd_terminal_slave_);

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
