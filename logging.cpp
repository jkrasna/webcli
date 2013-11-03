/*
 * Logging.cpp
 *
 *  Created on: Oct 5, 2013
 *      Author: jkrasna
 */

#include "logging.h"

#include "main.h"

Logging::Logging(std::string file_name) {
	log_level_ = LOG_LEVEL_VERBOSE;
	open_ = true;

	log_file_ = (std::ofstream *)new std::ofstream();
	if(!log_file_) {
		open_ = false;
		//die((int)DIE_RESOURCE_ALLOCATION);
	}
}

Logging::~Logging() {
	if(log_file_) {
		if(log_file_->is_open()) {
			log_file_->close();
		}
		delete log_file_;
	}
}

