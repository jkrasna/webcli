#include "main.h"
#include "web_interface.h"
#include "application_data.h"
#include "application_interface.h"
#include "log_sink.h"
#include "log_sink_file.h"
#include "logging.h"

int main(int argc, char **argv)
{
	Logging::add_sink(new LogSinkFile(LL_ALL, "main.log"));

	std::shared_ptr<ApplicationData> application_data(new ApplicationData("/home/jkrasna/", "nc", true));

	application_data->addNewArgument("-l");
	application_data->addNewArgument("-p");
	application_data->addNewArgument("5000");

	std::shared_ptr<ApplicationInterface> application_interface(new ApplicationInterface(application_data));
	std::shared_ptr<WebInterface> web_interface(new WebInterface(application_interface));

	web_interface.get()->process();

	web_interface.reset();
	application_data.reset();
	application_interface.reset();
	return 0;
}



