#include "main.h"
#include "control_flow.h"
#include "log_sink.h"
#include "log_sink_file.h"
#include "logging.h"

int main(int argc, char **argv)
{
	Logging::add_sink(new LogSinkFile(LL_ALL, "main.log"));

	ControlFlow *cf = new ControlFlow();
	cf->run();
	delete cf;

	return 0;
}



