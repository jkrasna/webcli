#include "application_interface.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stropts.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>

#include "logging.h"

#define MILI_TO_MICRO(mili) 	mili*1000

ApplicationInterface::ApplicationInterface(
		std::shared_ptr<ApplicationData> application_data) {
	initialize(application_data);
}

void ApplicationInterface::initialize(
		std::shared_ptr<ApplicationData> application_data) {
	application_data_ 	= application_data;

	running_			= true;
	stop_flag_			= false;

	message_index_ 		= 0;
	input_messages_ 	= new std::deque<ConsoleLinePtr>();
	output_messages_ 	= new std::deque<ConsoleLinePtr>();

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

void ApplicationInterface::stop() {
	stop_flag_ = true;

	LOG_DBG("Stopping application-interface thread");

	if(worker_thread_) {
		worker_thread_->join();
	}

	LOG_DBG("Application-interface thread stopped");

}

void ApplicationInterface::add_input_message(std::string message) {
	std::lock_guard<std::mutex> _(*mutex_);
	ConsoleLinePtr ptr(new ConsoleLine(input_messages_->size() + 1, message.c_str()));
	input_messages_->push_back(ptr);
}

void ApplicationInterface::add_input_message(char *message) {
	std::lock_guard<std::mutex> _(*mutex_);
	ConsoleLinePtr ptr(new ConsoleLine(input_messages_->size() + 1, message));
	input_messages_->push_back(ptr);
}

ConsoleLinePtrDequePtr ApplicationInterface::get_output_message(int last_index) {
	std::lock_guard<std::mutex> _(*mutex_);
	ConsoleLinePtrDequePtr queue(new ConsoleLinePtrDeque());

	for(ConsoleLinePtr line_ptr : *output_messages_) {
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
		int rc = chdir(application_data_->get_run_path());
		errno_save = errno;

		if (rc < 0) {
			LOG_ERR("Failed to change directory! ERRNO = %d", errno_save);
			return applicationError;
		}

		// Replace exiting child process with what we want to execute
		if (application_data_->is_search_enabled()) {
			// If we do not know the path of the application we can let
			// the system search for it in the standard paths
			execvp(application_data_->get_application(),
					(char **)application_data_->get_argument_list());
			errno_save = errno;
		} else {
			// This requires that the getApplication method returns the
			// full path of the application
			execv(application_data_->get_application(),
					(char **)application_data_->get_argument_list());
			errno_save = errno;
		}

		LOG_CRT("ERROR: Unable to start: '%s' - exec failed! ERRNO = %d",
				application_data_->get_application(), errno_save);

		return applicationError;
	} else {
		// This is the PARENT process
		LOG_DBG("Child: PID = %d, SID = %d!", child_pid_, child_sid_);

		// Close slave terminal as we do not need it anymore
		close(fd_terminal_slave_);
	}

	return applicationSuccess;
}

void ApplicationInterface::worker() {
	bool changed = false;
	int count = 0;
	int rc = 0;
	std::unique_ptr<char> buffer;

	if (this->start_subprocess() != applicationSuccess) {
		LOG_CRT("Failed to start subprocess!");
		running_ = false;
		return;
	}

	// Allocate buffer for reading messages
	buffer.reset(new char[READ_LIMIT + 1]);
	if(!buffer) {
		LOG_CRT("Failed to allocate output message buffer!");
		stop_flag_ = true;
	}

	LOG_TRC("APPINTF_THRD: Before while loop!");

	while (!stop_flag_) {
		// Write all messages sent by the user to terminal
		if(input_messages_->size() > 0) {
			std::lock_guard<std::mutex> _(*mutex_);

			ConsoleLinePtr message_pointer = input_messages_->front();
			ConsoleLine *message = message_pointer.get();

			LOG_TRC("Writing message: '%s'", message->get_line());

			// Write message to terminal connecting child application
			count = write(fd_terminal_master_, (void *)message->get_line(), message->get_size());
			if(count < 0) {
				LOG_ERR("Write failed for message: '%s'", message->get_line());
			} else if(((unsigned long)count) != (message->get_size() * sizeof(char))) {
				LOG_WRN("Not all bytes written! %d of %lu", count, message->get_size());
			}

			input_messages_->pop_front();
			message_pointer.reset();
			changed = true;
		}

		count 	= 0;
		rc 		= 0;

		// Read any messages posted by the application
		do {
			// Create the write file descriptor set and add file descriptor to it
			fd_set read_set;
			FD_ZERO(&read_set);
			FD_SET(fd_terminal_master_, &read_set);

			// Data in the timeout struct will change during select so we need to define it
			// each time before the call to select function with the desired microsecond value
			struct timeval timeout;
			timeout.tv_sec  = 0;
			timeout.tv_usec = MILI_TO_MICRO(100);

			// Wait for for the file descriptor to become ready for reading or for the timeout
			// to expire
			rc = select(fd_terminal_master_ + 1, &read_set, NULL, NULL, &timeout);
			if(rc < 0) {
				LOG_ERR("Select failed on terminal master file descriptor, ERRNO = %d", errno);
			}
			else if(rc > 0 && FD_ISSET(fd_terminal_master_, &read_set)) {

				// Clear message buffer
				memset((void *)buffer.get(), 0, READ_LIMIT + 1);

				// Read message from the child application
				count = read(fd_terminal_master_, (void *)buffer.get(), READ_LIMIT);
				if(count < 0) {
					LOG_ERR("Failed to read anything from the master terminal after select!");
					break;
				}
				(buffer.get())[count + 1] = 0;

				// Lock the read message mutex
				std::lock_guard<std::mutex> _(*mutex_);

				// Add message to output message queue
				LOG_DBG("New message read: %s", buffer.get());
				output_messages_->emplace_back(new ConsoleLine(message_index_++, buffer.get()));

				changed = true;
			}
		} while(rc > 0);

		// The timeout expired - no (more) messages

		if(!changed) {
			// Nothing was read or written
			usleep(100);
		}
	}

	LOG_TRC("APPINTF_THRD: After while loop!");

	// Free message buffer
	buffer.reset();

	if(child_pid_ > 1) {
		LOG_DBG("Killing child process!");
		kill(child_pid_, SIGTERM);
	}
}
