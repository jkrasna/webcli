/*
 * template.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: jkrasna
 */

#include "web_template.h"
#include "logging.h"

#define DEFAULT_PAGE 	"<html>\n"\
						"\t<head>\n"\
						"\t\t<title>{{TITLE}}</title>\n"\
						"\t\t{{HEAD}}\n"\
						"\t</head>\n"\
						"\t<body>\n"\
						"\t\t<h1>{{TITLE}}"\
						"\t\t{{MENU}}\n"\
						"\t\t<hr>\n"\
						"\t\t{{CONTENT}}\n"\
						"\t</body>\n"\
						"</html>"
#define DEFAULT_REGEX	"\\{\\{([^\\}]+)\\}\\}"

WebTemplate::WebTemplate() {
	LOG_TRC("%s", __func__);
	template_ = new std::string(DEFAULT_PAGE);
	pattern_ = new boost::regex(DEFAULT_REGEX);
	parse_ok_ = true;
}

WebTemplate::WebTemplate(std::string filename) {
	LOG_TRC("%s", __func__);
	template_ = new std::string();
	pattern_ = new boost::regex(DEFAULT_REGEX);
	parse_ok_ = parse(filename);
}

WebTemplate::~WebTemplate() {
	delete template_;
	delete pattern_;
}

StringPtr WebTemplate::get_with_content(std::map<std::string, StringPtr> content) {

	StringPtr final(new std::string());

	std::string::const_iterator start;
	std::string::const_iterator end;
	start = (*template_).begin();
	end   = (*template_).end();
	std::string final_end;

	boost::match_results<std::string::const_iterator> what;
	boost::match_flag_type flags = boost::match_default;

	LOG_TRC("TEMPLATE 0");
	if (!pattern_) {
		LOG_TRC("TEMPLATE 0 - PATTERN ERROR");
	}

	while (boost::regex_search(start, end, what, *pattern_, flags)) {

		LOG_TRC("TEMPLATE 1");
		std::map<std::string, StringPtr>::iterator it;
		final->append(what.prefix());
		if ((it = content.find(what[1].str())) != content.end()) {
			final->append(*it->second);
		}

		LOG_TRC("TEMPLATE 2");
		// update search position:
		start = what[0].second;
		// update flags:
		flags |= boost::match_prev_avail;
		flags |= boost::match_not_bob;

		final_end = what.suffix().str();
	}

	LOG_TRC("TEMPLATE 3");

	final->append(final_end);

	return final;
}

bool WebTemplate::parse_ok() {
	return parse_ok_;
}

bool WebTemplate::parse(std::string filename) {
	LOG_TRC("%s", __func__);

	// Open input template file
	std::ifstream input(filename, std::ios::in | std::ios::binary);
	if (input) {
		LOG_DBG("Successfully opened file %s for reading!", filename.c_str());

		// Calculate file size
		unsigned long int size = 0;
		input.seekg(0, std::ios::end);
		size = input.tellg();
		input.seekg(0, std::ios::beg);

		LOG_DBG("Successfully calculated file size: %luB", size);

		if(size > template_->max_size()) {
			LOG_ERR("Template file size exceeds maximum string size!");
		} else {
			// Reserve the space in the string object beforehand
			template_->reserve(size);

			LOG_TRC("Reserving space suceeded!");

			template_->assign((std::istreambuf_iterator<char>(input)),
						std::istreambuf_iterator<char>());
		}

		// Close input template file
		input.close();

		return true;
	}

	LOG_ERR("Failed to parse template file!");
	return false;
}
