#ifndef WEBCLI_WEB_INTERFACE_H_
#define WEBCLI_WEB_INTERFACE_H_

#include "shared.h"

#include "application_interface.h"

class WebInterface {
public:
	WebInterface(std::shared_ptr<ApplicationInterface> application_interface);
	virtual ~WebInterface(void);

	inline bool is_running() {
		return running_;
	}

	void stop();

private:
	void worker();

	std::shared_ptr<ApplicationInterface> application_interface_;

	std::thread *worker_thread_;
	std::mutex *mutex_;

	volatile bool running_;
	volatile bool stop_flag_;
};

#endif /* WEBCLI_WEB_INTERFACE_H_ */
