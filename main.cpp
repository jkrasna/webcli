#include "main.h"

#include "web_interface.h"
#include "application_interface.h"

#include <boost/smart_ptr.hpp>

#include <iostream>
#include <stdio.h>

using namespace boost;

int main(int argc, char **argv)
{
	boost::shared_ptr<ApplicationInterface> application_interface(new ApplicationInterface());
	boost::shared_ptr<WebInterface> web_interface(new WebInterface(application_interface.get()));

	web_interface.get()->process();

	web_interface.reset();
	application_interface.reset();
	return 0;
}



