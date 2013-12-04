/*
 * template.cpp
 *
 *  Created on: Nov 10, 2013
 *      Author: jkrasna
 */

#include "web_template.h"
#include "logging.h"
#include "utils.h"

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
	template_ = Utils::read_file(filename).get();
	pattern_ = new boost::regex(DEFAULT_REGEX);
	parse_ok_ = (template_ != NULL);
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

	while (boost::regex_search(start, end, what, *pattern_, flags)) {

		std::map<std::string, StringPtr>::iterator it;
		final->append(what.prefix());
		if ((it = content.find(what[1].str())) != content.end()) {
			final->append(*it->second);
		}

		// update search position:
		start = what[0].second;
		// update flags:
		flags |= boost::match_prev_avail;
		flags |= boost::match_not_bob;

		final_end = what.suffix().str();
	}
	final->append(final_end);

	return final;
}

bool WebTemplate::parse_ok() {
	return parse_ok_;
}
