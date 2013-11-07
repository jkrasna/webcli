#include "control_flow.h"

ControlFlow::ControlFlow() {
	application_data_.reset(new ApplicationData("/home/jkrasna/", "nc", true));

	application_data_->add_new_argument((char *)"-l");
	application_data_->add_new_argument((char *)"-p");
	application_data_->add_new_argument((char *)"5000");

	application_interface_.reset(new ApplicationInterface(application_data_));
	web_interface_.reset(new WebInterface(application_interface_));
}

ControlFlow::~ControlFlow() {
	application_data_.reset();
	application_interface_.reset();
	web_interface_.reset();
}

void ControlFlow::run() {
	if(web_interface_) {
		web_interface_->process();
	}
}

