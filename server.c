#include "server.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "logger.h"
#include "listener.h"

enum http_status {
	HTTP_200,
	HTTP_201,
	HTTP_202,
	HTTP_204,
	HTTP_301,
	HTTP_302,
	HTTP_304,
	HTTP_400,
	HTTP_401,
	HTTP_403,
	HTTP_404,
	HTTP_500,
	HTTP_501,
	HTTP_502,
	HTTP_503,
};

static struct http_status_code {
	unsigned short status;
	char *reason;
} http_status_codes[] = {
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 204, "No Content" },
	{ 301, "Moved Permanently" },
	{ 302, "Moved Temporarily" },
	{ 304, "Not Modified" },
	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
};

ssize_t aprintf(char **buffer, const char *format, ...)
{
	ssize_t length;
	va_list ap;

	va_start(ap, format);
	length = vsnprintf(NULL, 0, format, ap);
	va_end(ap);

	if ((*buffer = malloc(length + 1)) == NULL) {
		eerror("Failed to allocate memory");
		return -1;
	}

	va_start(ap, format);
	vsnprintf(*buffer, length + 1, format, ap);
	va_end(ap);

	return length;
}

static int write_response(int client, enum http_status status)
{
	struct http_status_code http = http_status_codes[status];
	ssize_t length;
	char *response;

	if ((length = aprintf(&response,
	                      "HTTP/1.0 %u %s\r\n"
	                      "\r\n"
	                      "%u %s",
	                      http.status, http.reason,
	                      http.status, http.reason)) < 0) {
		error("Failed to format response");
		return -1;

	} else if (write(client, response, length) < 0) {
		eerror("Failed to write");
		free(response);
		return -1;
	}

	dump("response", response, length);
	free(response);
	return 0;
}

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
			write_response(client, HTTP_404);
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
