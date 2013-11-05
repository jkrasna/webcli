#include "application_interface.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "logging.h"

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
	std::deque<consoleLinePtr> * queue = new std::deque<consoleLinePtr>();

	for(consoleLinePtr line_ptr : *output_messages_) {
		queue->push_back(line_ptr);
	}

	return queue;
}

//! The function that starts the application as a subprocess

//! This function opens the pseudo-terminal master and then forks another
//! process with the child pseudo-terminal set. It then returns allowing
//! for communication between the terminals.
int ApplicationInterface::start_subprocess() {
	int errno_save = 0;

	// Open master pseudo-terminal
	fd_terminal_master_ = posix_openpt(O_RDWR | O_NOCTTY);
	errno_save = errno;

	if (fd_terminal_master_ < 0) {
		LOG_ERR("Failed to open master pseudo-terminal! ERRNO = %d", errno_save);
	}

	// Change permissions of the pseudo terminal file and unlock slave side
	if( grantpt (fd_terminal_master_) == -1
			|| unlockpt (fd_terminal_master_) == -1
			|| (slave_name_ = ptsname (fd_terminal_master_)) == NULL) {
		errno_save = errno;
		LOG_ERR("Failed to unlock master pseudo-terminal! ERRNO = %d", errno_save);
	}

	LOG_DBG("slave device is: %s\n", slave_name_);

	// Open slave terminal - needed for child application
	fd_terminal_slave_ = open(slave_name_, O_RDWR | O_NOCTTY);
	errno_save = errno;

	if (fd_terminal_slave_ < 0) {
		LOG_ERR("Failed to open slave pseudo-terminal! ERRNO = %d", errno_save);
		close(fd_terminal_master_);
		return applicationError;
	}

	// Load pseudo terminal control modules "ptem" and "ldterm" to slave terminal

	if (ioctl(fd_terminal_slave_, I_PUSH, "ptem") == -1) {
		errno_save = errno;
		LOG_WRN("Failed to load 'ptem' slave module! ERRNO = %d", errno_save);
	}

	if (ioctl(fd_terminal_slave_, I_PUSH, "ldterm") == -1) {
		errno_save = errno;
		LOG_WRN("Failed to load 'ldterm' slave module! ERRNO = %d", errno_save);
	}

	// Creating a new subprocess
	child_pid_ = fork();
	errno_save = errno;

	// Delegate functionality depending on the process
	if (child_pid_ < 0) {

		LOG_ERR("Failed to fork child process! ERRNO = %d", errno_save);
		close(fd_terminal_master_);
		close(fd_terminal_slave_);
		return applicationError;

	} else if (child_pid_ == 0) {
		// This is the CHILD process

		// Close master pseudo-terminal file descriptor
		close(fd_terminal_master_);

		// Connect slave terminal to input output and error file descriptors
		if( dup2(fd_terminal_slave_, 0) == -1
				|| dup2(fd_terminal_slave_, 1) == -1
				|| dup2(fd_terminal_slave_, 2) == -1 ) {
			errno_save = errno;
			LOG_ERR("Failed to reassign slave FDs! ERRNO = %d", errno_save);

			close(fd_terminal_slave_);
			return applicationError;
		}

		// Close slave terminal as we do not need it anymore
		close(fd_terminal_slave_);

		// Create new session id for the process group
		if ((child_sid_ = setsid()) == -1) {
			errno_save = errno;
			LOG_ERR("Failed to create a new session ID! ERRNO = %d", errno_save);
			return applicationError;
		}

		// Change working directory to the configured one
		int rc = chdir(application_data_->getRunPath());
		errno_save = errno;

		if (rc < 0) {
			LOG_ERR("Failed to change directory! ERRNO = %d", errno_save);
			return applicationError;
		}

		// Replace exiting child process with what we want to execute
		if (application_data_->isSearchEnabled()) {
			// If we do not know the path of the application we can let
			// the system search for it in the standard paths
			execvp(application_data_->getApplication(),
					application_data_->getArgumentList());
			errno_save = errno;
		} else {
			// This requires that the getApplication method returns the
			// full path of the application
			execv(application_data_->getApplication(),
					application_data_->getArgumentList());
			errno_save = errno;
		}

		LOG_CRT("ERROR: Unable to start: '%s' - exec failed! ERRNO = %d",
				application_data_->getApplication(), errno_save);

		return applicationError;
	} else {
		// This is the PARENT process

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
			// Sleep for a while - give the other threads a chance
			//std::this_thread::sleep(1);
			sleep(1);
		}
	}
}
