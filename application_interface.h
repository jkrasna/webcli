#ifndef WEBCLI_APPLICATION_INTERFACE_H_
#define WEBCLI_APPLICATION_INTERFACE_H_

#include "precompiled.h"
#include "application_data.h"
#include "application_console_line.h"

#define QUEUE_MESSAGE_COUNT_MAX 	1024;

enum APPLICATION_STATUS {
	applicationSuccess, applicationError,
};

class ApplicationInterface {
public:
	ApplicationInterface(std::shared_ptr<ApplicationData> application_data);
	virtual ~ApplicationInterface();

	void addApplicationInputMessage(std::string);
	void addApplicationInputMessage(char *);
	std::deque<ApplicationConsoleLinePtr> *getApplicationOutputMessage(
			int last_index);

private:
	void initialize(std::shared_ptr<ApplicationData> application_data);
	void worker();

	int start_subprocess();

	std::thread *worker_thread_;

	std::deque<ApplicationConsoleLinePtr> *application_input_messages_;
	std::deque<ApplicationConsoleLinePtr> *application_output_messages_;

	std::shared_ptr<ApplicationData> application_data_;
};

#endif /* WEBCLI_APPLICATION_INTERFACE_H_ */
