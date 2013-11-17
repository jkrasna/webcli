#ifndef WEBCLI_TEMPLATE_H_
#define WEBCLI_TEMPLATE_H_

#include "shared.h"

class WebTemplate {
public:
	WebTemplate();
	WebTemplate(std::string filename);
	virtual ~WebTemplate();

	StringPtr get_with_content(std::map<std::string, StringPtr> content);

	bool parse_ok();

private:
	bool parse(std::string filename);

	bool parse_ok_;

	std::string *template_;
	boost::regex *pattern_;
};

#endif /* WEBCLI_TEMPLATE_H_ */
