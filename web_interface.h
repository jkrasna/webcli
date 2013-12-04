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
	void initialize();

	StringPtr generate_main_page();
	StringPtr generate_cljson_before(std::string time);
	StringPtr generate_cljson_after(std::string time);
	StringPtr generate_command_json(bool success);
	StringPtr generate_status_page();
	StringPtr generate_log_page();
	StringPtr generate_lljson_before(std::string time);
	StringPtr generate_lljson_after(std::string time);
	StringPtr generate_error_page();

	bool add_command(std::string command);

	StringStreamPtr format_line(ConsoleLinePtrDequePtr console_messages);
	StringStreamPtr format_json(ConsoleLinePtrDequePtr console_messages);
	StringStreamPtr format_menu(std::string page);
	StringStreamPtr format_head(std::string time_after, std::string time_before, std::string json_page);

	std::map<std::string, std::string> parseURI(std::string uri);
	std::string get_home_folder_path(std::string filename);
	std::string if_file_get_mime(std::string uri, std::string *path);

	std::shared_ptr<ApplicationInterface> application_interface_;

	std::string home_folder_;
	std::string page_title_;
	std::string template_name_;

	boost::regex *uri_regex_;
	boost::regex *uri_regex_query_;
	boost::regex *uri_regex_file_;

	std::mutex  *mutex_;
	std::thread *worker_thread_;

	std::map<std::string, std::string> *menu_;

	//WebTemplate *web_template_;

	volatile bool running_;
	volatile bool stop_flag_;
};

#endif /* WEBCLI_WEB_INTERFACE_H_ */
