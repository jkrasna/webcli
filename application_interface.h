#ifndef WEBCLI_APPLICATION_INTERFACE_H_
#define WEBCLI_APPLICATION_INTERFACE_H_

#include "precompiled.h"
#include "application_data.h"

enum APPLICATION_STATUS {
	applicationSuccess, applicationError,
};

struct APPLICATION_MESSAGE {
	int index;
	std::string data;
};

class ApplicationInterface {
public:
	ApplicationInterface(std::shared_ptr<ApplicationData> application_data);
	virtual ~ApplicationInterface();

	void addApplicationInputMessage(std::string);
	std::deque<APPLICATION_MESSAGE> *getApplicationOutputMessage(
			int last_index);

	int start();

private:
	void initialize(std::shared_ptr<ApplicationData> application_data);
	void worker();

	std::thread *worker_thread_;

	std::deque<APPLICATION_MESSAGE> *application_input_messages_;
	std::deque<APPLICATION_MESSAGE> *application_output_messages_;

	std::shared_ptr<ApplicationData> application_data_;
};

#endif /* WEBCLI_APPLICATION_INTERFACE_H_ */
