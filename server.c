#include "server.h"

#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logger.h"
#include "listener.h"

static int listener = -1;

int start_server(const char *interface, const char *service)
{
	struct sockaddr sa;
	socklen_t salen = sizeof(sa);
	char buffer[1024];
	ssize_t length;
	int client;

	if ((listener = get_listener(interface, service)) < 0) {
		error("Failed to get listener");
		return -1;
	}

	while (listener > 0) {
		if ((client = accept(listener, &sa, &salen)) < 0) {
			if (errno == EINTR) {
				return 0;

			} else {
				eerror("Failed to accept client");
				return -1;
			}
		}

		debug("Got a new connection");

		if ((length = read(client, buffer, sizeof(buffer))) < 0) {
			eerror("Failed to read");

		} else {
			dump("recv", buffer, length);
		}

		if (close(client) < 0) {
			ewarning("Failed to close client");
		}
	}

	return 0;
}

int stop_server(void)
{
	return close(listener);
}
