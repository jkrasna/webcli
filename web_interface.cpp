/*
 * web_interface.cpp
 *
 *  Created on: Oct 6, 2013
 *      Author: jkrasna
 */

#include "web_interface.h"

#include <iostream>

#include <fcgio.h>
#include <fcgiapp.h>

#include "application_interface.h"

using namespace std;

WebInterface::WebInterface(ApplicationInterface *interface) {
	application_interface_ = (ApplicationInterface *) interface;
}

WebInterface::~WebInterface() {

}

void WebInterface::process() {
	// Backup the stdio streambufs
	streambuf * cin_streambuf = cin.rdbuf();
	streambuf * cout_streambuf = cout.rdbuf();
	streambuf * cerr_streambuf = cerr.rdbuf();

	FCGX_Request request;

	cout << "Before FCGI init!" << endl;

	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);

	while (FCGX_Accept_r(&request) == 0) {
		fcgi_streambuf cin_fcgi_streambuf(request.in);
		fcgi_streambuf cout_fcgi_streambuf(request.out);
		fcgi_streambuf cerr_fcgi_streambuf(request.err);

		cin.rdbuf(&cin_fcgi_streambuf);
		cout.rdbuf(&cout_fcgi_streambuf);
		cerr.rdbuf(&cerr_fcgi_streambuf);

		cout << "Content-type: text/html\r\n" << "\r\n" << "<html>\n"
				<< "  <head>\n" << "    <title>Hello, World!</title>\n"
				<< "  </head>\n" << "  <body>\n"
				<< "    <h1>Hello, World!</h1>\n" << "  </body>\n"
				<< "</html>\n";

		// Note: the fcgi_streambuf destructor will auto flush
	}

	cout << "After response sent!" << endl;

	// restore stdio streambufs
	cin.rdbuf(cin_streambuf);
	cout.rdbuf(cout_streambuf);
	cerr.rdbuf(cerr_streambuf);
}

