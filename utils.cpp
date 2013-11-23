/*
 * utils.cpp
 *
 *  Created on: Nov 21, 2013
 *      Author: jkrasna
 */

#include "utils.h"
#include "logging.h"

Utils::Utils() {
	// TODO Auto-generated constructor stub

}

Utils::~Utils() {
	// TODO Auto-generated destructor stub
}

std::string *Utils::read_file(std::string filename) {
	LOG_TRC("%s", __func__);

	std::string *file_content = new std::string;

	// Open input template file
	std::ifstream input(filename, std::ios::in | std::ios::binary);
	if (file_content && input.is_open() && input.good()) {
		LOG_DBG("Successfully opened file %s for reading!", filename.c_str());

		// Calculate file size
		unsigned long int size = 0;
		input.seekg(0, std::ios::end);
		size = input.tellg();
		input.seekg(0, std::ios::beg);

		LOG_DBG("Successfully calculated file size: %luB", size);

		if(size > file_content->max_size()) {
			LOG_ERR("File size exceeds maximum string size!");
		} else {
			// Reserve the space in the string object beforehand
			file_content->reserve(size);

			LOG_TRC("Reserving space suceeded!");

			file_content->assign((std::istreambuf_iterator<char>(input)),
						std::istreambuf_iterator<char>());
		}

		// Close input template file
		input.close();

		return file_content;
	}

	LOG_ERR("Failed to parse file: %s", filename.c_str());
	return NULL;
}
