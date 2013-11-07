#ifndef WEBCLI_APPLICATION_DATA_H_
#define WEBCLI_APPLICATION_DATA_H_

#include "shared.h"

#define READ_LIMIT		255

typedef std::shared_ptr<std::string> stringPtr;

class ApplicationData {
public:
	ApplicationData(std::string run_path, std::string application, bool search=false);
	virtual ~ApplicationData();

	void add_new_argument(char *argument);
	void add_new_argument(std::string argument);

	const char *get_run_path();
	const char *get_application();
	bool is_search_enabled();
	const char **get_argument_list();

private:
	void initialize(std::string run_path, std::string application, bool search);

	std::string *run_path_;
	std::string *application_;
	bool search_;

	std::vector<stringPtr> *arguments_;
	std::mutex *mutex_;

	const char **argument_list_;
	bool arguments_changed_;
};

#endif /* WEBCLI_APPLICATION_DATA_H_ */
