#include "application_interface.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <stdio.h>
#include <string.h>

ApplicationInterface::ApplicationInterface(
		std::shared_ptr<ApplicationData> application_data) {
	initialize(application_data);
}

void ApplicationInterface::initialize(
		std::shared_ptr<ApplicationData> application_data) {
	application_data_ 	= application_data;

	message_index_ 		= 0;
	input_messages_ 	= new std::deque<consoleLinePtr>();
	output_messages_ 	= new std::deque<consoleLinePtr>();

	mutex_ = new std::mutex();

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

	delete mutex_;
	delete input_messages_;
	delete output_messages_;

	application_data_.reset();
}

void ApplicationInterface::addInputMessage(std::string message) {
	std::lock_guard<std::mutex> _(*mutex_);
	consoleLinePtr ptr(new ConsoleLine(input_messages_->size() + 1, message.c_str()));
	input_messages_->push_back(ptr);
}

void ApplicationInterface::addInputMessage(char *message) {
	std::lock_guard<std::mutex> _(*mutex_);
	consoleLinePtr ptr(new ConsoleLine(input_messages_->size() + 1, message));
	input_messages_->push_back(ptr);
}

std::deque<consoleLinePtr> *ApplicationInterface::getOutputMessage(int last_index) {
	std::lock_guard<std::mutex> _(*mutex_);
	std::deque<consoleLinePtr> * q = new std::deque<consoleLinePtr>();

	for(int i = 0; i < output_messages_->size(); i++) {
		q->push_back(output_messages_->at(i));
	}

	return q;
}

//! The function that starts the application as a subprocess

//! This function opens the pseudoterminal master and then forks another
//! process with the child pseudoterminal set. It then returns allowing
//! for communication between the terminals.
int ApplicationInterface::start_subprocess() {

	// Open master pseudoterminal
	fd_terminal_master_ = posix_openpt(O_RDWR | O_NOCTTY);

	// Change permissions of the pseudo terminal file and unlock slave side
	if (fd_terminal_master_ < 0|| grantpt (fd_terminal_master_) == -1
			|| unlockpt (fd_terminal_master_) == -1
			|| (slave_name_ = ptsname (fd_terminal_master_)) == NULL) {
		// TODO: Log error here
		printf("ERROR: Failed to open pseudo-terminal!");
	}

	// TODO: Change into log
	printf("slave device is: %s\n", slave_name_);

	// Open slave terminal - needed for child application
	fd_terminal_slave_ = open(slave_name_, O_RDWR | O_NOCTTY);
	if (fd_terminal_slave_ < 0) {
		// TODO: Log error here
		printf("ERROR: Failed to pseudo-terminal slave!");
		close(fd_terminal_master_);
		return applicationError;
	}

	// Load pseudo terminal control modules "ptem" and "ldterm" to slave terminal
	if (ioctl(fd_terminal_slave_, I_PUSH, "ptem") == -1) {
		printf("WARNING: Failed to load 'ptem' slave module!");
	}

	if (ioctl(fd_terminal_slave_, I_PUSH, "ldterm") == -1) {
		printf("WARNING: Failed to load 'ldterm' slave module!");
	}

	// Creating a new subprocess
	child_pid_ = fork();

	// Delegate functionality depending on the process
	if (child_pid_ < 0) {
		//TODO: Fork failed - log message
		printf("ERROR: Failed to load fork process!");
		close(fd_terminal_master_);
		close(fd_terminal_slave_);
		return applicationError;

	} else if (child_pid_ == 0) {
		// This is the child process

		// Close master pseudoterminal file descriptor
		close(fd_terminal_master_);

		// Connect slave terminal to input output and error fds
		if( dup2(fd_terminal_slave_, 0) == -1
				|| dup2(fd_terminal_slave_, 1) == -1
				|| dup2(fd_terminal_slave_, 2) == -1 ) {

			printf("ERROR: Failed to reassign FDs!");
			close(fd_terminal_slave_);
			return applicationError;
		}

		// Close slave terminal as we do not need it anymore
		close(fd_terminal_slave_);

		// Create new session id for the process group
		if ((child_sid_ = setsid()) == -1) {
			// TODO: Error - unable to create new session
			printf("ERROR: Failed to new session ID!");
			return applicationError;
		}

		// Change working directory to the configured one
		int rc = chdir(application_data_->getRunPath());
		if (rc < 0) {
			// TODO: Error - unable to change directory to specified dir
			printf("ERROR: Failed change directory to expected location!");
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

		char str[40];
		sprintf(str, "Child PID = %d!", child_pid_);
		std::lock_guard<std::mutex> _(*mutex_);
		consoleLinePtr message_pointer(new ConsoleLine(message_index_++, str));
						output_messages_->push_back(message_pointer);
	}

	return applicationSuccess;
}

void ApplicationInterface::worker() {
	bool running = true;
	bool changed = false;
	int count = 0;

	if ((count = this->start_subprocess()) != applicationSuccess) {
		//TODO: throw initialization error
		char str[40];
		sprintf(str, "Subprocess error! RC = %d!", count);
		consoleLinePtr message_pointer(new ConsoleLine(message_index_++, str));
								output_messages_->push_back(message_pointer);
		// TODO: Change to logging
		//printf("ERROR: Failed to start subprocess!");
	}

	while (running) {
		//TODO: Write messages to terminal
		if(input_messages_->size() > 0) {
			std::lock_guard<std::mutex> _(*mutex_);

			consoleLinePtr message_pointer = input_messages_->front();
			ConsoleLine *message = message_pointer.get();

			count = write(fd_terminal_master_, (void *)message->getLine(), message->getSize());
			if(count < 0) {
				// TODO: Error: write failed
			} else if(count != (message->getSize() * sizeof(char))) {
				// TODO: Warning: Not all bytes written
			}

			input_messages_->pop_front();
			message_pointer.reset();
			changed = true;
		}

		{
			std::lock_guard<std::mutex> _(*mutex_);

			std::shared_ptr<char> buffer((char *)calloc(READ_LIMIT + 1, sizeof(char)));
			if(!buffer.get()) {
				// TODO: ERROR: Failed to allocate message buffer
				continue;
			}

			count = 0;
			do {
				//TODO: Read messages from terminal
				count = read(fd_terminal_master_, (void *)buffer.get(), READ_LIMIT);
				if(count < 0) {
					// TODO: Error: failed to read anything from the master terminal
					break;
				}

				consoleLinePtr message_pointer(new ConsoleLine(message_index_++, buffer.get()));
				output_messages_->push_back(message_pointer);

				memset((void *)buffer.get(), 0, READ_LIMIT + 1);

				changed = true;

			} while (count > 0);

			buffer.reset();
		}

		if(!changed) {
			// TODO: Sleep for a while - give the other threads a chance
			//std::this_thread::sleep(1);
			sleep(1);
		}
	}
}
