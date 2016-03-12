#include "control_flow.h"

#include "logging.h"

#include <unistd.h>
#include <signal.h>

ControlFlow::ControlFlow() {
	running_ = true;
	//application_data_.reset(new ApplicationData("/home/jkrasna", "/home/jkrasna/workspace/constprint/cprnt", false));
	application_data_.reset(new ApplicationData("/home/jkrasna", "sh", true));

	//application_data_->add_new_argument((char *)"-l");
	//application_data_->add_new_argument((char *)"-p");
	//application_data_->add_new_argument((char *)"5000");

	application_interface_.reset(new ApplicationInterface(application_data_));
	web_interface_.reset(new WebInterface(application_interface_));
}

ControlFlow::~ControlFlow() {
	application_data_.reset();
	application_interface_.reset();
	web_interface_.reset();
}

void ControlFlow::run() {
	while(running_) {
		usleep(100);
	}

	LOG_DBG("Stopping control flow main loop!");

	web_interface_->stop();
	application_interface_->stop();
}

void ControlFlow::process_signal(int signum) {
	switch(signum) {
	case SIGTERM:
	case SIGPIPE:
	case SIGUSR1:
		running_ = false;
		LOG_DBG("Signal received: %d", signum);
		break;
	default:
		LOG_WRN("Unknown signal received: %d", signum);
	}
}

