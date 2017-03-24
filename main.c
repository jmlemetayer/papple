#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "logger.h"
#include "server.h"

static void usage(void)
{
	fprintf(stderr,
	        "Usage: " PACKAGE_NAME " [OPTIONS]...\n\n"
	        "-i, --interface IF      Use the specified interface to bind.\n"
	        "                          (default is " DEFAULT_INTERFACE ")\n"
	        "-s, --service SRV       Use the specified service to bind.\n"
	        "                          (default is " DEFAULT_SERVICE ")\n"
	        "    --version           Display version.\n"
	        "    --help              Display this help screen.\n\n"
	        PACKAGE_NAME " home page: <" PACKAGE_URL ">\n"
	        "Report " PACKAGE_NAME " bugs to <" PACKAGE_BUGREPORT ">\n");
}

static void signal_handler(int signal)
{
	switch (signal) {
	case SIGINT:
	case SIGQUIT:
	case SIGTERM:
		notice("Stopping " PACKAGE_NAME);
		stop_server();
		break;
	}
}

static int handle_signals(void)
{
	struct sigaction signal_action;

	signal_action.sa_handler = signal_handler;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = 0;

	if (sigaction(SIGINT, &signal_action, NULL) < 0) {
		perror("Failed to handle SIGINT");
		return -1;

	} else if (sigaction(SIGQUIT, &signal_action, NULL) < 0) {
		perror("Failed to handle SIGQUIT");
		return -1;

	} else if (sigaction(SIGTERM, &signal_action, NULL) < 0) {
		perror("Failed to handle SIGTERM");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;
	char *interface = NULL;
	char *service = NULL;
	int opt, optidx;

	const struct option lopt[] = {
		{"version",    no_argument,        NULL,  0 },
		{"help",       no_argument,        NULL,  0 },
		{"interface",  required_argument,  NULL, 'i'},
		{"service",    required_argument,  NULL, 's'},
		{NULL,         0,                  NULL,  0 },
	};

	while ((opt = getopt_long(argc, argv, "i:s:", lopt, &optidx)) != EOF) {
		switch (opt) {
		case 'i':
			if (interface == NULL &&
			    (interface = strdup(optarg)) == NULL) {
				perror("Failed to duplicate interface");
			}

			break;

		case 's':
			if (service == NULL &&
			    (service = strdup(optarg)) == NULL) {
				perror("Failed to duplicate service");
			}

			break;

		case 0:
			if (optidx == 0) {
				fprintf(stdout, PACKAGE_STRING "\n");
				status = EXIT_SUCCESS;
				goto exit;
			}

		default:
			usage();
			goto exit;
		}
	}

	if (interface == NULL &&
	    (interface = strdup(DEFAULT_INTERFACE)) == NULL) {
		perror("Failed to duplicate interface");
		goto exit;

	} else if (service == NULL &&
	           (service = strdup(DEFAULT_SERVICE)) == NULL) {
		perror("Failed to duplicate service");
		goto exit;

	} else if (handle_signals() < 0) {
		goto exit;
	}

#ifdef WITH_SYSLOG
	openlog(PACKAGE_NAME, LOG_PERROR | LOG_PID, LOG_USER);
#endif

	notice("Starting " PACKAGE_STRING);

	if (start_server(interface, service) < 0) {
		error("Failed to start server");

	} else {
		status = EXIT_SUCCESS;
	}

exit:
	free(interface);
	free(service);
	exit(status);
}
