#ifndef WEBCLI_WEB_INTERFACE_H_
#define WEBCLI_WEB_INTERFACE_H_

#include "shared.h"

class ApplicationInterface;

class WebInterface {
public:
	WebInterface(std::shared_ptr<ApplicationInterface> application_interface);
	virtual ~WebInterface(void);

	void process(void);
private:
	std::shared_ptr<ApplicationInterface> application_interface_;

	std::thread *worker_thread_;
	std::mutex *mutex_;
};

#endif /* WEBCLI_WEB_INTERFACE_H_ */
