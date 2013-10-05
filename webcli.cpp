/*
 * main.cpp
 *
 *  Created on: Sep 14, 2013
 *      Author: jkrasna
 */

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>

#define NULL_FILE "/dev/null"

void printHelp();
void printVersion();

char   *webcliName;            // The name of this executable (server)

int main(int argc, char **argv)
{
	webcliName = argv[0];

	return 0;
}

void printHelp()
{
	printf("Usage: %s [options] <port> <command args ...>    (-h for help)\n"
           "<port>              use telnet <port> for command connections\n"
           "<command args ...>  command line to start child process\n"
           "Options:\n"
           "    --allow             allow control connections from anywhere\n"
           "    --autorestartcmd    command to toggle auto restart flag (^ for ctrl)\n"
           "    --coresize <n>      set maximum core size for child to <n>\n"
           " -c --chdir <dir>       change directory to <dir> before starting child\n"
           " -d --debug             debug mode (keeps child in foreground)\n"
           " -e --exec <str>        specify child executable (default: arg0 of <command>)\n"
           " -f --foreground        keep child in foreground (interactive)\n"
           " -h --help              print this message\n"
           "    --holdoff <n>       set holdoff time between child restarts\n"
           " -i --ignore <str>      ignore all chars in <str> (^ for ctrl)\n"
           " -k --killcmd <str>     command to kill (reboot) the child (^ for ctrl)\n"
           "    --killsig <n>       signal to send to child when killing\n"
           " -l --logport <n>       allow log connections through telnet port <n>\n"
           " -L --logfile <file>    write log to <file>\n"
           "    --logstamp [<str>]  prefix log lines with timestamp [strftime format]\n"
           " -n --name <str>        set child's name (default: arg0 of <command>)\n"
           "    --noautorestart     do not restart child on exit by default\n"
           " -p --pidfile <str>     name of PID file (for server PID)\n"
           " -q --quiet             suppress informational output (server)\n"
           "    --restrict          restrict log connections to localhost\n"
           "    --timefmt <str>     set time format (strftime) to <str>\n"
           " -V --version           print program version\n"
           " -w --wait              wait for telnet cmd to manually start child\n"
           " -x --logoutcmd <str>   command to logout client connection (^ for ctrl)\n"
        );
}

void printVersion()
{
    printf(PROCSERV_VERSION_STRING "\n");
}

void deamonize()
{
    pid_t pid;
    int fh;

    pid = fork();
    if (pid < 0) {
    	perror("Unable to fork daemon process");
        exit(errno);
    }
    else if (pid > 0) {      // PARENT (foreground command)
        if (!quiet) {
            fprintf(stderr, "%s: spawning daemon process: %ld\n", procservName, (long) pid);
            if (-1 == logFileFD) {
                fprintf(stderr, "Warning: No log file%s specified.\n",
                        logPort ? "" : " and no port for log connections");
            }
        }
        exit(0);

    } else {                 // CHILD (background daemon)
        procservPid = getpid();

        // Redirect stdin, stdout, stderr to /dev/null
        char buf[] = "/dev/null";
        fh = open(NULL_FILE, O_RDWR);
        if (fh < 0) { perror(NULL_FILE); exit(-1); }
        close(0); close(1); close(2);
        dup(fh); dup(fh); dup(fh);
        close(fh);

        // Make sure we are not attached to a terminal
        setsid();
    }
}



