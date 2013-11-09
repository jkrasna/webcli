#include "web_interface.h"

#include <fcgio.h>
#include <fcgiapp.h>
#include <stdlib.h>
#include <unistd.h>

#include "application_interface.h"
#include "logging.h"

using namespace std;

WebInterface::WebInterface(std::shared_ptr<ApplicationInterface> application_interface) {
	running_ 	= true;
	stop_flag_ 	= false;

	application_interface_ = application_interface;

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
	streambuf * cin_streambuf  = cin.rdbuf();
	streambuf * cout_streambuf = cout.rdbuf();
	streambuf * cerr_streambuf = cerr.rdbuf();

	FCGX_Request request;

	LOG_TRC("Before FCGI init!");

	FCGX_Init();
	FCGX_InitRequest(&request, 0, FCGI_FAIL_ACCEPT_ON_INTR);

	std::deque<consoleLinePtr> * console_messages = NULL;

	LOG_TRC("After FCGI init!");

	while (!stop_flag_) {
		if(FCGX_Accept_r(&request) >= 0) {
			fcgi_streambuf cin_fcgi_streambuf(request.in);
			fcgi_streambuf cout_fcgi_streambuf(request.out);
			fcgi_streambuf cerr_fcgi_streambuf(request.err);

			LOG_TRC("Processing request!");

			cin.rdbuf(&cin_fcgi_streambuf);
			cout.rdbuf(&cout_fcgi_streambuf);
			cerr.rdbuf(&cerr_fcgi_streambuf);

			cout 	<< "Content-type: text/html\r\n"
					<< "\r\n"
					<< "<html>\n"
					<< "\t<head>\n"
					<< "\t\t<title>Hello, World!</title>\n"
					<< "\t</head>\n"
					<< "\t<body>\n"
					<< "\t\t<h1>Hello, World!</h1>\n";

			const char * uri = FCGX_GetParam("REQUEST_URI", request.envp);
			cout << "URI: " << uri << endl;

			LOG_TRC("Before requesting messages from app interface!");
			console_messages = application_interface_->get_output_message(0);
			LOG_TRC("After requesting messages from app interface!");
			if (console_messages->size() == 0) {
				cout << "\t\t<p>Empty messages!</p>\n";
			}
			else {
				cout << "\t\t<table style='border:1px solid #CCC;'>\n";
				//for (int i = 0; i < console_messages->size(); i++) {
				for (consoleLinePtr &console_line : *console_messages) {
					cout << "\t\t<tr><td>" << console_line->get_index() << "</td><td>" << console_line->get_line() << "</td></tr>\n";
				}
				cout << "\t\t</table>\n";
			}

			console_messages->clear();
			delete console_messages;

			cout << "<form action='" << uri << "' method='post'>"
				 << "<input type='text' name='command'>"
				 << "<input type='submit' value='Send'>"
				 << "</form>";

			cout 	<< "\t</body>\n"
					<< "</html>\n";

			// Note: the fcgi_streambuf destructor will auto flush
		}
		else {
			LOG_TRC("FCGX_Aceept_r returned less than 0!");
		}
	}

	LOG_TRC("Out of accept request loop!");

	// Free request
	FCGX_Finish_r(&request);

	// Flag the thread as not running anymore.
	running_ = false;

	// restore stdio streambufs
	cin.rdbuf(cin_streambuf);
	cout.rdbuf(cout_streambuf);
	cerr.rdbuf(cerr_streambuf);
}

