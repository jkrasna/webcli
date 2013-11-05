/*
 * web_interface.cpp
 *
 *  Created on: Oct 6, 2013
 *      Author: jkrasna
 */

#include "web_interface.h"

#include <fcgio.h>
#include <fcgiapp.h>

#include "application_interface.h"

using namespace std;

WebInterface::WebInterface(std::shared_ptr<ApplicationInterface> application_interface) {
	application_interface_ = application_interface;
}

WebInterface::~WebInterface() {
	application_interface_.reset();
}

void WebInterface::process() {
	// Backup the stdio streambufs
	streambuf * cin_streambuf  = cin.rdbuf();
	streambuf * cout_streambuf = cout.rdbuf();
	streambuf * cerr_streambuf = cerr.rdbuf();

	FCGX_Request request;

	cout << "Before FCGI init!" << endl;

	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);

	std::deque<consoleLinePtr> * console_messages = NULL;
	consoleLinePtr mp = NULL;

	while (FCGX_Accept_r(&request) == 0) {
		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);

		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);

		console_messages = application_interface_->getOutputMessage(0);

		cout 	<< "Content-type: text/html\r\n"
				<< "\r\n"
				<< "<html>\n"
				<< "\t<head>\n"
				<< "\t\t<title>Hello, World!</title>\n"
				<< "\t</head>\n"
				<< "\t<body>\n"
				<< "\t\t<h1>Hello, World!</h1>\n";

		if (console_messages->size() == 0) {
			cout << "\t\t<p>Empty messages!</p>\n";
		}
		else {
			cout << "\t\t<table>\n";
			for (int i = 0; i < console_messages->size(); i++) {
				mp = console_messages->at(i);
				cout << "\t\t<tr><td>" << mp->getIndex() << "</td><td>" << mp->getLine() << "</td></tr>\n";
			}
			cout << "\t\t</table>\n";
		}

		cout 	<< "\t</body>\n"
				<< "</html>\n";

		// Note: the fcgi_streambuf destructor will auto flush
	}

	cout << "After response sent!" << endl;

	// restore stdio streambufs
	cin.rdbuf(cin_streambuf);
	cout.rdbuf(cout_streambuf);
	cerr.rdbuf(cerr_streambuf);
}

