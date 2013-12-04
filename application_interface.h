#ifndef WEBCLI_APPLICATION_INTERFACE_H_
#define WEBCLI_APPLICATION_INTERFACE_H_

#include "shared.h"
#include "application_data.h"
#include "console_line.h"

#define QUEUE_MESSAGE_COUNT_MAX 	1024

enum APPLICATION_STATUS {
	applicationSuccess, applicationError,
};

class ApplicationInterface {
public:
	ApplicationInterface(ApplicationDataPtr application_data);
	virtual ~ApplicationInterface();

	void add_input_message(std::string);
	void add_input_message(char *);
	ConsoleLinePtrDequePtr get_output_message();
	ConsoleLinePtrDequePtr get_output_message_before(std::string iso_time);
	ConsoleLinePtrDequePtr get_output_message_after(std::string iso_time);

	inline bool is_running() {
		return running_;
	}

	void stop();

private:
	void initialize(ApplicationDataPtr application_data);
	void worker();

	int start_subprocess();

	bool running_;
	bool stop_flag_;

	std::thread *worker_thread_;
	std::mutex *mutex_;

	std::deque<ConsoleLinePtr> *input_messages_;
	std::deque<ConsoleLinePtr> *output_messages_;

	std::shared_ptr<ApplicationData> application_data_;

	pid_t child_pid_;
	pid_t child_sid_;
	int fd_terminal_master_;
	int fd_terminal_slave_;
	char *slave_name_;
};

#endif /* WEBCLI_APPLICATION_INTERFACE_H_ */
