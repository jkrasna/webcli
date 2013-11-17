#include "web_interface.h"

#include <fcgio.h>
#include <fcgiapp.h>
#include <stdlib.h>
#include <unistd.h>

#include "application_interface.h"
#include "logging.h"

#define KW_TITLE 	"TITLE"
#define KW_HEAD		"HEAD"
#define KW_MENU		"MENU"
#define KW_CONTENT  "CONTENT"

WebInterface::WebInterface(std::shared_ptr<ApplicationInterface> application_interface) {
	running_ 	= true;
	stop_flag_ 	= false;

	application_interface_ = application_interface;

	web_template_ = new WebTemplate("template/template.html");

	mutex_ = new std::mutex();
	worker_thread_ = new std::thread(&WebInterface::worker, this);
}

WebInterface::~WebInterface() {
	stop_flag_ = true;
	if(worker_thread_) {
		delete worker_thread_;
	}

	application_interface_.reset();
	delete mutex_;
}

void WebInterface::stop() {
	stop_flag_ = true;
	FCGX_ShutdownPending();

	LOG_DBG("Stopping web-interface thread");

	if(worker_thread_) {
		worker_thread_->join();
	}

	LOG_DBG("Web-interface thread stopped");
}

void WebInterface::worker() {
	// Backup the stdio streambufs
	std::streambuf * cin_streambuf  = std::cin.rdbuf();
	std::streambuf * cout_streambuf = std::cout.rdbuf();
	std::streambuf * cerr_streambuf = std::cerr.rdbuf();

	const std::string kw_title(KW_TITLE);
	const std::string kw_head(KW_HEAD);
	const std::string kw_menu(KW_MENU);
	const std::string kw_content(KW_CONTENT);


	FCGX_Request request;

	// Initialize FastCGI library and request
	FCGX_Init();
	FCGX_InitRequest(&request, 0, FCGI_FAIL_ACCEPT_ON_INTR);

	LOG_DBG("FastCGI initialization success!");

	// TODO: Read this from the configuration database / file
	StringPtr page_title(new std::string("WebCLI"));

	while (!stop_flag_) {
		if(FCGX_Accept_r(&request) >= 0) {

			fcgi_streambuf cin_fcgi_streambuf(request.in);
			fcgi_streambuf cout_fcgi_streambuf(request.out);
			fcgi_streambuf cerr_fcgi_streambuf(request.err);

			std::cin.rdbuf(&cin_fcgi_streambuf);
			std::cout.rdbuf(&cout_fcgi_streambuf);
			std::cerr.rdbuf(&cerr_fcgi_streambuf);

			// getting the uri from the request
			std::string uri;
			const char *uri_param = FCGX_GetParam("REQUEST_URI", request.envp);
			if(!uri_param) {
				LOG_ERR("Failed to retrieve the request URI environment value!");
				uri = "error";
			} else {
				uri = uri_param;
			}

			LOG_DBG("Request received: %s", uri.c_str());

			// Prepare template content
			std::map<std::string, StringPtr> template_data;
			template_data[kw_title] = page_title;

			if(uri.compare("/") == 0 || uri.empty()) {
				LOG_TRC("START PAGE");

				StringPtr content = format_content(
						application_interface_->get_output_message(0));
				template_data[kw_content] = content;
				template_data[kw_menu] = format_menu();
				template_data[kw_head] = format_head();

				StringPtr page = web_template_->get_with_content(template_data);
				std::cout << *(page.get());

			} else if (uri.compare("error") == 0) {
				LOG_TRC("ERROR PAGE");
			}
		}
		else {
			LOG_TRC("FCGX_Aceept_r returned less than 0!");
		}
	}

	LOG_TRC("Out of accept request loop!");

	// Free request strucure
	FCGX_Finish_r(&request);

	// Flag the thread as not running anymore.
	running_ = false;

	// restore stdio streambufs
	std::cin.rdbuf(cin_streambuf);
	std::cout.rdbuf(cout_streambuf);
	std::cerr.rdbuf(cerr_streambuf);
}

StringPtr WebInterface::format_content(ConsoleLinePtrDequePtr console_messages) {
	std::stringstream data;

	data << "\t\t\t<div id=\"output\">\n";

	if (console_messages->size() > 0) {
		for (ConsoleLinePtr &console_line : *console_messages) {
			data << "\t\t\t\t<div id=\"line" << console_line->get_index() << "\" class=\"line\">";
			data << console_line->get_line();
			data << "</div>" << std::endl;
		}
	}
	console_messages.reset();

	data << "\t\t\t</div>" << std::endl;
	data << "\t\t\t<div id=\"input\">" << std::endl;
	data << "\t\t\t\t<form action=\"\\\" autocomplete=\"off\">" << std::endl;
	data << "\t\t\t\t\t<input id=\"command\" type=\"text\" name=\"command\">" << std::endl;
	data << "\t\t\t\t\t<input id=\"submit\" type=\"submit\" value=\"Submit\">" << std::endl;
	data << "\t\t\t\t</form>" << std::endl;
	data << "\t\t\t</div>" << std::endl;

	StringPtr content(new std::string(data.str()));
	return content;
}

StringPtr WebInterface::format_menu() {
	std::stringstream data;

	data << "\t\t<ul>" << std::endl;
	data << "\t\t\t<li><a class=\"selected\" href=\"#\">Console</a></li>" << std::endl;
	data << "\t\t\t<li><a href=\"config\">Configuration</a></li>" << std::endl;
	data << "\t\t\t<li class=\"last\"><a href=\"log\">Log output</a></li>" << std::endl;
	data << "\t\t</ul>" << std::endl;

	StringPtr menu(new std::string(data.str()));
	return menu;
}

StringPtr WebInterface::format_head() {
	std::stringstream data;

	data << "" << std::endl;

	StringPtr head(new std::string(data.str()));
	return head;
}

