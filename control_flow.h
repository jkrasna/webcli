#ifndef WEBCLI_CONTROL_FLOW_H_
#define WEBCLI_CONTROL_FLOW_H_

#include "shared.h"

#include "web_interface.h"
#include "application_data.h"
#include "application_interface.h"

class ControlFlow {
public:
	ControlFlow();
	virtual ~ControlFlow();

	void run();

private:
	std::shared_ptr<ApplicationData> application_data_;
	std::shared_ptr<ApplicationInterface> application_interface_;
	std::shared_ptr<WebInterface> web_interface_;
};

#endif /* WEBCLI_RUN_FLOW_H_ */
