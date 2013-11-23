#ifndef WEBCLI_WEB_INTERFACE_H_
#define WEBCLI_WEB_INTERFACE_H_

#include "shared.h"

#include "application_interface.h"
#include "web_template.h"

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

	StringPtr format_content(ConsoleLinePtrDequePtr console_messages);
	StringPtr format_menu();
	StringPtr format_head();

	std::string get_mime_type(std::string filetype);

	std::shared_ptr<ApplicationInterface> application_interface_;

	std::thread *worker_thread_;
	std::mutex *mutex_;

	WebTemplate *web_template_;

	volatile bool running_;
	volatile bool stop_flag_;
};

#endif /* WEBCLI_WEB_INTERFACE_H_ */
