#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "logger.h"
#include "listener.h"

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

int main(int argc, char **argv)
{
	int status = EXIT_FAILURE;
	char *interface = NULL;
	char *service = NULL;
	int listener = -1;
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
	}

#ifdef WITH_SYSLOG
	openlog(PACKAGE_NAME, LOG_PERROR | LOG_PID, LOG_USER);
#endif

	notice("Starting " PACKAGE_STRING);

	if ((listener = get_listener(interface, service)) < 0) {
		error("Failed to get listener");
		goto exit;
	}

	status = EXIT_SUCCESS;
exit:
	close(listener);
	free(interface);
	free(service);
	exit(status);
}
