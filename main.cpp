#include "main.h"
#include "control_flow.h"
#include "log_sink.h"
#include "log_sink_file.h"
#include "logging.h"

#include <signal.h>

static ControlFlow *g_cf = NULL;

void process_signal(int signum) {
	if(g_cf) {
		g_cf->process_signal(signum);
	}
	else {
		LOG_WRN("Received signal (%d) when control flow object not initialized!", signum);
	}
}

int main(int argc, char **argv)
{
	Logging::add_sink(new LogSinkFile(LL_ALL, "main.log"));

	g_cf = new ControlFlow();

	signal(SIGPIPE, &process_signal);
	signal(SIGTERM, &process_signal);
	signal(SIGUSR1, &process_signal);

	g_cf->run();

	LOG_DBG("Control flow thread stopped");

	delete g_cf;

	LOG_DBG("EXITING...");

	return 0;
}



