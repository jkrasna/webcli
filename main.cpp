#include "main.h"
#include "precompiled.h"
#include "web_interface.h"
#include "application_data.h"
#include "application_interface.h"

int main(int argc, char **argv)
{
	std::shared_ptr<ApplicationData> application_data(new ApplicationData("/home/jkrasna/", "ls"));

	application_data->addNewArgument("-al");

	std::shared_ptr<ApplicationInterface> application_interface(new ApplicationInterface(application_data));
	std::shared_ptr<WebInterface> web_interface(new WebInterface(application_interface.get()));

	web_interface.get()->process();

	web_interface.reset();
	application_data.reset();
	application_interface.reset();
	return 0;
}



