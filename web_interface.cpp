#include "web_interface.h"

#include <fcgio.h>
#include <fcgiapp.h>
#include <stdlib.h>
#include <unistd.h>

#include "application_interface.h"
#include "logging.h"
#include "utils.h"

#include <ctemplate/template.h>

#define KW_TITLE 	"TITLE"
#define KW_HEAD		"HEAD"
#define KW_MENU		"MENU"
#define KW_CONTENT  "CONTENT"

#define KW_SECTION_INPUT "INPUT"

#define URI_PAGE			std::string("page")
#define URI_PAGE_MAIN		std::string("/")			/* Main page */
#define URI_PAGE_LOG		std::string("/log")		/* Log message page */
#define URI_PAGE_STATUS 	std::string("/status")	/* Status page */
#define URI_PAGE_ERROR		std::string("/error")	/* Error page */
#define URI_PAGE_CL			std::string("/cl") 		/* Console lines JSON */
#define URI_PAGE_LL			std::string("/ll")		/* Log lines JSON */
#define URI_PAGE_BEFORE		std::string("before")
#define URI_PAGE_AFTER		std::string("after")
#define URI_PAGE_COMMAND	std::string("command")
#define URI_PAGE_SOURCE		std::string("source")

#define URI_REGEX_PAGE	"(/*[^\\?/]*)\\??"						/* Reg. expr. to match page part of the URI (before first ?) */
#define URI_REGEX_QSTR	"\\&?([^\\=]+)=([^\\&]*)"				/* Reg. expr. to match each part of the query string &<PRM>=<VALUE> */
#define URI_REGEX_FILE  "/?[A-Za-z0-9\\-_/]+\\.([A-Za-z0-9]+)" 	/* Reg. expr. to check if URI is a file. Each file must have an extension. */

WebInterface::WebInterface(std::shared_ptr<ApplicationInterface> application_interface) {
	running_ 		= true;
	stop_flag_ 		= false;

	application_interface_ = application_interface;

	//web_template_ 	= new WebTemplate("template/template.html");

	uri_regex_ 			= new boost::regex(URI_REGEX_PAGE);
	uri_regex_query_ 	= new boost::regex(URI_REGEX_QSTR);
	uri_regex_file_		= new boost::regex(URI_REGEX_FILE);

	menu_ 			= new std::map<std::string, std::string>();

	mutex_ 			= new std::mutex();
	worker_thread_ 	= new std::thread(&WebInterface::worker, this);

	initialize();
}

void WebInterface::initialize() {
	home_folder_ 	= "./template";
	page_title_  	= "WebCLI";
	template_name_ 	= "template.html";

	(*menu_)[URI_PAGE_MAIN] 	= "Main";
	(*menu_)[URI_PAGE_LOG] 		= "Log";
	(*menu_)[URI_PAGE_STATUS]	= "Status";
}

WebInterface::~WebInterface() {
	stop_flag_ = true;
	application_interface_.reset();
}

void WebInterface::stop() {
	stop_flag_ = true;
	FCGX_ShutdownPending();

	LOG_DBG("Stopping web-interface thread");
	if(worker_thread_) {
		worker_thread_->join();
		delete worker_thread_;
	}

	LOG_DBG("Web-interface thread stopped");

	delete mutex_;
	delete uri_regex_;
	delete uri_regex_query_;
	delete uri_regex_file_;
}

void WebInterface::worker() {
	/* Backup the stdio streambufs */
	std::streambuf * cin_streambuf  = std::cin.rdbuf();
	std::streambuf * cout_streambuf = std::cout.rdbuf();
	std::streambuf * cerr_streambuf = std::cerr.rdbuf();

	const std::string kw_title(KW_TITLE);
	const std::string kw_head(KW_HEAD);
	const std::string kw_menu(KW_MENU);
	const std::string kw_content(KW_CONTENT);

	FCGX_Request request;

	/* Initialize FastCGI library and request */
	FCGX_Init();
	FCGX_InitRequest(&request, 0, FCGI_FAIL_ACCEPT_ON_INTR);

	LOG_DBG("FastCGI initialization success!");

	while (!stop_flag_) {
		if(FCGX_Accept_r(&request) >= 0) {

			fcgi_streambuf cin_fcgi_streambuf(request.in);
			fcgi_streambuf cout_fcgi_streambuf(request.out);
			fcgi_streambuf cerr_fcgi_streambuf(request.err);

			std::cin.rdbuf(&cin_fcgi_streambuf);
			std::cout.rdbuf(&cout_fcgi_streambuf);
			std::cerr.rdbuf(&cerr_fcgi_streambuf);

			/* getting the uri from the request */
			std::string uri;
			const char *uri_param = FCGX_GetParam("REQUEST_URI", request.envp);
			if(!uri_param) {
				LOG_ERR("Failed to retrieve the request URI environment value!");
				uri = URI_PAGE_ERROR;
			} else {
				uri = uri_param;
			}

			LOG_DBG("Request received: %s", uri.c_str());

			/* Check if URI is a file in the home folder and get the mime of
			 * that file (by extension) */
			std::string path;
			std::string mime = if_file_get_mime(uri, &path);

			if (!mime.empty()) {
				/* This is a file we need to serve */
				StringPtr file_data = Utils::read_file(path);
				std::cout << "Content-type: " << mime << "\r\n\r\n";
				std::cout << *(file_data);

				file_data.reset();
			} else {
				/* Parse the URI */
				std::map<std::string, std::string> uri_data = parseURI(uri);

				LOG_DBG("URI Parsed, page requested: %s",
						uri_data[URI_PAGE].c_str());

				/* Generate and serve the page depending on the URI */
				StringPtr page;
				std::string content_type = "text/html";

				/* Main page requested */
				if (uri_data[URI_PAGE].compare(URI_PAGE_MAIN) == 0) {
					bool success = false;

					/* Check if a command was sent from the client. */
					if (uri_data.find(URI_PAGE_COMMAND) != uri_data.end()) {
						success = add_command(uri_data[URI_PAGE_COMMAND]);
					}

					std::string s;
					/* Check if the request was sent from javascript or pure HTML */
					if(uri_data.find(URI_PAGE_SOURCE) != uri_data.end()) {
						LOG_DBG("This query's source IS javascript: %s, Source: %s",
								uri.c_str(), (uri_data[URI_PAGE_SOURCE]).c_str());
						content_type = "application/json";
						page = generate_command_json(success);
					} else {
						LOG_DBG("This query's source IS NOT javascript: %s", uri.c_str());
						/* Just generate a standard main page */
						page = generate_main_page();
					}
				/* Log page requested */
				} else if (uri_data[URI_PAGE].compare(URI_PAGE_LOG) == 0) {
					page = generate_log_page();

				/* Status page requested */
				} else if (uri_data[URI_PAGE].compare(URI_PAGE_STATUS) == 0) {
					page = generate_status_page();

				/* Console lines JSON page requested */
				} else if (uri_data[URI_PAGE].compare(URI_PAGE_CL) == 0) {
					if (uri_data.find(URI_PAGE_BEFORE) != uri_data.end()) {
						content_type = "application/json";
						page = generate_cljson_before(
								uri_data[URI_PAGE_BEFORE]);
					} else if (uri_data.find(URI_PAGE_AFTER)
							!= uri_data.end()) {
						content_type = "application/json";
						page = generate_cljson_after(uri_data[URI_PAGE_AFTER]);
					} else {
						page = generate_error_page();
					}

				/* Log lines JSON page requested */
				} else if (uri_data[URI_PAGE].compare(URI_PAGE_LL) == 0) {
					if (uri_data.find(URI_PAGE_BEFORE) != uri_data.end()) {
						content_type = "application/json";
						page = generate_lljson_before(
								uri_data[URI_PAGE_BEFORE]);
					} else if (uri_data.find(URI_PAGE_AFTER)
							!= uri_data.end()) {
						content_type = "application/json";
						page = generate_lljson_after(uri_data[URI_PAGE_AFTER]);
					} else {
						page = generate_error_page();
					}
				} else {
					page = generate_error_page();
				}

				/* Output the generated page with the correct content type */
				std::cout << "Content-type: " << content_type << "\r\n\r\n";
				std::cout << *(page.get());
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

StringPtr WebInterface::generate_main_page() {
	StringPtr page(new std::string);
	ctemplate::TemplateDictionary dict("main");

	LOG_DBG("Generating main page!");

	ConsoleLinePtrDequePtr lines = application_interface_->get_output_message();

	dict[KW_TITLE] 		= page_title_ + " - Console interface";
	dict[KW_HEAD] 		= (format_head(lines->front()->get_time(), lines->back()->get_time(), URI_PAGE_CL)->str());
	dict[KW_CONTENT] 	= (format_line(lines)->str());
	dict[KW_MENU] 		= (format_menu(URI_PAGE_MAIN)->str());

	dict.ShowSection(KW_SECTION_INPUT);

	LOG_DBG("Expanding template: %s", get_home_folder_path(template_name_).c_str());

	ctemplate::ExpandTemplate(get_home_folder_path(template_name_),
			ctemplate::DO_NOT_STRIP, &dict, page.get());

	LOG_DBG("Main page size: %lu", page->size());

	return page;
}

StringPtr WebInterface::generate_cljson_before(std::string time) {
	StringPtr page(new std::string);

	LOG_DBG("Generating console line JSON before %s", time.c_str());

	ConsoleLinePtrDequePtr lines = application_interface_->get_output_message_before(time);
	page->append(format_json(lines)->str());

	return page;
}

StringPtr WebInterface::generate_cljson_after(std::string time) {
	StringPtr page(new std::string);

	LOG_DBG("Generating console line JSON after %s", time.c_str());

	ConsoleLinePtrDequePtr lines = application_interface_->get_output_message_after(time);
	page->append(format_json(lines)->str());

	return page;
}

StringPtr WebInterface::generate_command_json(bool success) {
	StringPtr page(new std::string);

	page->append("{\"success\":\"");
	page->append(success?"true\"}":"false\"}");

	return page;
}

StringPtr WebInterface::generate_status_page() {
	StringPtr page(new std::string());
	ctemplate::TemplateDictionary dict("status");

	LOG_DBG("Generating status page!");

	std::stringstream content;
	content << "<div id=\"status\">" << std::endl;
	content << "<h1>Status</h1>" << std::endl;
	content << "<table id=\"status-table\">" << std::endl;

	content << "<tr>" << std::endl;
	content << "<th>Parameter</th>" << std::endl;
	content << "<th>Value</th>" << std::endl;
	content << "</tr>" << std::endl;

	content << "<tr>" << std::endl;
	content << "<td>Title</td>" << std::endl;
	content << "<td>" << page_title_ << "</td>" << std::endl;
	content << "</tr>" << std::endl;

	content << "<tr>" << std::endl;
	content << "<td>Home folder</td>" << std::endl;
	content << "<td>" << home_folder_ << "</td>" << std::endl;
	content << "</tr>" << std::endl;

	content << "<tr>" << std::endl;
	content << "<td>Template</td>" << std::endl;
	content << "<td>" << template_name_ << "</td>" << std::endl;
	content << "</tr>" << std::endl;

	content << "<tr>" << std::endl;
	content << "<td>Application running</td>" << std::endl;
	content << "<td>" << ((application_interface_->is_running())?"Yes":"No");
	content << "</td>" << std::endl;
	content << "</tr>" << std::endl;

	content << "</table>" << std::endl;
	content << "</div>\n" << std::endl;

	dict[KW_TITLE] 		= page_title_ + " - Status page";
	dict[KW_HEAD] 		= (format_head("", "", "")->str());
	dict[KW_CONTENT] 	= content.str();
	dict[KW_MENU] 		= (format_menu(URI_PAGE_STATUS)->str());

	ctemplate::ExpandTemplate(get_home_folder_path(template_name_),
			ctemplate::DO_NOT_STRIP, &dict, page.get());

	LOG_DBG("Status page size: %lu", page->size());

	return page;
}

StringPtr WebInterface::generate_log_page() {
	StringPtr page(new std::string());
	ctemplate::TemplateDictionary dict("log");

	LOG_DBG("Generating log page!");

	dict[KW_TITLE] 		= page_title_ + " - Logging output";
	dict[KW_HEAD] 		= (format_head("", "", "")->str());
	dict[KW_CONTENT] 	= ("Log page content");
	dict[KW_MENU] 		= (format_menu(URI_PAGE_LOG)->str());

	ctemplate::ExpandTemplate(get_home_folder_path(template_name_),
			ctemplate::DO_NOT_STRIP, &dict, page.get());

	LOG_DBG("Log page size: %lu", page->size());

	return page;
}

StringPtr WebInterface::generate_lljson_before(std::string time) {
	StringPtr page(new std::string);

	LOG_DBG("Generating log line JSON before %s", time.c_str());

	//TODO: Call get log lines function and compile it into JSON.
	//ConsoleLinePtrDequePtr lines = application_interface_->get_output_message_before(time);
	//page->append(format_json(lines)->str());

	return page;
}

StringPtr WebInterface::generate_lljson_after(std::string time) {
	StringPtr page(new std::string);

	LOG_DBG("Generating log line JSON after %s", time.c_str());

	//TODO: Call get log lines function and compile it into JSON.
	//ConsoleLinePtrDequePtr lines = application_interface_->get_output_message_after(time);
	//page->append(format_json(lines)->str());

	return page;
}

StringPtr WebInterface::generate_error_page() {
	StringPtr page(new std::string());
	ctemplate::TemplateDictionary dict("error");

	LOG_DBG("Generating error page!");

	dict[KW_TITLE] 		= page_title_ + " - Error";
	dict[KW_HEAD] 		= (format_head("", "", "")->str());
	dict[KW_CONTENT] 	= "Error page content!";
	dict[KW_MENU] 		= (format_menu(URI_PAGE_MAIN)->str());

	dict.ShowSection(KW_SECTION_INPUT);

	ctemplate::ExpandTemplate(get_home_folder_path(template_name_),
			ctemplate::DO_NOT_STRIP, &dict, page.get());

	LOG_DBG("Error page size: %lu", page->size());

	return page;
}

bool WebInterface::add_command(std::string command) {

	std::stringstream decoded;

	LOG_DBG("Decoding command: %s", command.c_str());

	for(unsigned i=0; i<command.size(); i++) {
		if(command.at(i) == '+') {
			decoded << " ";
		} else if (command.at(i) == '%') {
			short ch;
			std::stringstream converter;
			converter << std::hex << command.substr(i+1, 2);
			converter >> ch;
			decoded << (char)ch;
			i+=2;
		} else {
			decoded << command.at(i);
		}
	}

	LOG_DBG("Command decoded: %s", decoded.str().c_str());

	application_interface_->add_input_message(decoded.str());
	return true;
}

StringStreamPtr WebInterface::format_line(ConsoleLinePtrDequePtr console_messages) {
	StringStreamPtr content(new std::stringstream());

	(*content) << "\t\t\t" << "<div id=\"output\">" << std::endl;
	if (console_messages->size() > 0) {
		for (ConsoleLinePtr &console_line : *console_messages) {
			(*content) << "\t\t\t\t" << "<div class=\"line\">";
			(*content) << "<span class=\"time\">" << console_line->get_time() << "</span>";
			(*content) << console_line->get_line();
			(*content) << "</div>" << std::endl;
		}
	}
	(*content) << "\t\t\t" << "</div>" << std::endl;
	console_messages.reset();

	return content;
}

StringStreamPtr WebInterface::format_json(ConsoleLinePtrDequePtr console_messages) {
	StringStreamPtr json(new std::stringstream());

	(*json) << "[";
	if (console_messages->size() > 0) {
		for (auto cl = console_messages->begin(); cl != console_messages->end(); cl++) {
			(*json) << "[";
			(*json) << "{\"time\":\"" << (*cl)->get_time() << "\"},";
			(*json) << "{\"line\":\"" << (*cl)->get_line() << "\"}";
			if(cl+1 == console_messages->end()) {
				(*json) << "]";
			} else {
				(*json) << "],";
			}
		}
	}
	(*json) << "]";
	return json;
}

StringStreamPtr WebInterface::format_menu(std::string page) {
	StringStreamPtr menu(new std::stringstream());

	(*menu) << "\t\t" << "<ul>" << std::endl;
	if (menu_->size() > 0) {
		//for (std::pair<std::string,std::string>& menu_line: (*menu_)) {
		for (auto& menu_line: (*menu_)) {
			(*menu) << "\t\t\t" << "<li><a ";
			if(menu_line.first.compare(page) == 0) {
				(*menu) << "class=\"selected\" ";
			}
			(*menu) << "href=\"" << menu_line.first << "\">";
			(*menu) << menu_line.second << "</a></li>" << std::endl;
		}
	}
	(*menu) << "\t\t" << "</ul>" << std::endl;

	return menu;
}

StringStreamPtr WebInterface::format_head(std::string time_before, std::string time_after, std::string json_page) {
	StringStreamPtr head(new std::stringstream());

	(*head) << "\t\t" << "<script>" << std::endl;
	if(time_after.empty() || json_page.empty()) {
		(*head) << "\t\t\t" << "var __disabled = true;" << std::endl;
	} else {
		(*head) << "\t\t\t" << "var __disabled = false;" << std::endl;
		(*head) << "\t\t\t" << "var url_json_before = '" << json_page << "?before=';" << std::endl;
		(*head) << "\t\t\t" << "var url_json_after  = '" << json_page << "?after=';"  << std::endl;
		(*head) << "\t\t\t" << "var url_time_before = '" << time_before << "';" << std::endl;
		(*head) << "\t\t\t" << "var url_time_after  = '" << time_after  << "';" << std::endl;
	}
	(*head) << "\t\t" << "</script>" << std::endl;

	return head;
}

std::map<std::string, std::string> WebInterface::parseURI(std::string uri) {
	std::map<std::string, std::string> content;
	std::string::const_iterator start = uri.begin();
	std::string::const_iterator end   = uri.end();
	std::string query_string;

	boost::match_results<std::string::const_iterator> results;
	boost::match_flag_type flags = boost::match_default;

	// Parse page name
	if (boost::regex_search(start, end, results, *uri_regex_, flags)) {
		content[URI_PAGE] = results[1].str();
		query_string = results.suffix();
	}

	// Parse query part of the URI
	start = query_string.begin();
	end   = query_string.end();
	while(boost::regex_search(start, end, results, *uri_regex_query_, flags)) {

		// Set query string key value pairs
		content[results[1].str()] = results[2].str();
		LOG_DBG("parseURI: new key-pair: [%s]=%s", results[1].str().c_str(),
				results[2].str().c_str());

		// update search position:
		start = results[2].second;
		// update flags:
		flags |= boost::match_prev_avail;
		flags |= boost::match_not_bob;
	}

	return content;
}

std::string WebInterface::get_home_folder_path(std::string filename) {
	std::string path;

	// Delete leading slashes and dots to prevent access to files out of home folder
	while(filename.front() == '/' || filename.front() == '.') {
		filename.erase(filename.begin());
	}

	path.append(home_folder_);
	path.append("/");
	path.append(filename);

	return path;
}

/// Check if URI points to an existing file and get the MIME type of the file.
/**
 * @returns The MIME type as a string. Returns an empty string if the
 *          file type isn't recognised / can't be accessed.
 */
std::string WebInterface::if_file_get_mime(std::string uri, std::string *path) {
	std::string::const_iterator start = uri.begin();
	std::string::const_iterator end   = uri.end();
	std::string extension;

	boost::match_results<std::string::const_iterator> results;
	boost::match_flag_type flags = boost::match_default;

	// Delete leading slashes and dots to prevent access to files out of home folder
	while(uri.front() == '/' || uri.front() == '.') {
		uri.erase(uri.begin());
	}

	// Parse page name
	if (boost::regex_search(start, end, results, *uri_regex_file_, flags)) {
		extension = results[1].str();
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
	}
	else {
		LOG_TRC("Sent string does not pass as a file path: '%s'", uri.c_str());
		return "";
	}
	path->clear();
	path->append(home_folder_);
	path->append("/");
	path->append(uri);

	boost::filesystem::path file_system_path((*path));

	// Check if requested file actually exists and is a regular file
	if(!boost::filesystem::exists(file_system_path) ||
			!boost::filesystem::is_regular_file(file_system_path))
	{
		LOG_WRN("Requested non-existing or non-regular file: %s", path->c_str());
		return "";
	}

	if (extension == "js") {
		return "application/javascript";
	} else if (extension == "json") {
		return "application/json";
	} else if (extension == "css") {
		return "text/css";
	} else if (extension == "html" || extension == "htm") {
		return "text/html";
	} else if (extension == "xml") {
		return "text/xml";
	} else if (extension == "csv") {
		return "text/csv";
	} else if (extension == "rtf") {
		return "text/rtf";
	} else if (extension == "jpg" || extension == "jpeg") {
		return "image/jpeg";
	} else if (extension == "gif") {
		return "image/gif";
	} else if (extension == "png") {
		return "image/png";
	} else if (extension == "zip") {
		return "application/zip";
	} else if (extension == "tar") {
		return "application/x-tar";
	} else {
		return ""; // Not allowed to download these file types.
	}
}
