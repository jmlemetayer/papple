#include "server.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logger.h"
#include "listener.h"

static int data_available(int client)
{
	struct timeval time;
	fd_set set;

	memset(&time, 0, sizeof(time));

	FD_ZERO(&set);
	FD_SET(client, &set);

	if (select(client + 1, &set, NULL, NULL, &time) < 0) {
		return -1;

	} else if (FD_ISSET(client, &set)) {
		return 1;

	} else {
		return 0;
	}
}

static ssize_t recv_request(int client, char **request)
{
	size_t size = 512;
	size_t len = 0;
	int available;
	char *req;
	ssize_t l;

	if ((*request = malloc(size)) == NULL) {
		eerror("Failed to allocate memory");
		return -1;
	}

	while (1) {
		if ((available = data_available(client)) < 0) {
			eerror("Failed to get data availability");
			free(*request);
			return -1;

		} else if (available == 0) {
			return len;

		} else if ((l = read(client, *request + len, size - len)) < 0) {
			eerror("Failed to read");
			free(*request);
			return -1;

		} else {
			len += l;
		}

		if (len >= size) {
			if ((req = realloc(*request, (size <<= 1))) == NULL) {
				eerror("Failed to reallocate memory");
				free(*request);
				return -1;

			} else {
				*request = req;
			}
		}
	}
}

static int listener = -1;

int start_server(const char *interface, const char *service)
{
	struct sockaddr sa;
	socklen_t salen = sizeof(sa);
	ssize_t length;
	char *request;
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

		if ((length = recv_request(client, &request)) < 0) {
			error("Failed to receive request");

		} else {
			dump("request", request, length);
			free(request);
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
