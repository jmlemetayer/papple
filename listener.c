#include "listener.h"

#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "logger.h"

static int get_addr_listener(const char *address, const char *service)
{
	struct addrinfo *addrs, *addr;
	struct addrinfo hints;
	int one = 1;
	int ret;

	memset(&hints, 0, sizeof(hints));

	if (address == NULL) {
		hints.ai_flags = AI_PASSIVE;
	}

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((ret = getaddrinfo(address, service, &hints, &addrs)) != 0) {
		error("Failed to get address info: %s", gai_strerror(ret));
		return -1;
	}

	for (addr = addrs; addr != NULL; addr = addr->ai_next) {
		if ((ret = socket(addr->ai_family, addr->ai_socktype,
		                  addr->ai_protocol)) < 0) {
			eerror("Failed to create socket");
			continue;

		} else if (setsockopt(ret, SOL_SOCKET, SO_REUSEADDR,
		                      &one, sizeof(one)) < 0) {
			eerror("Failed to set socket option: SO_REUSEADDR");
			continue;

		} else if (bind(ret, addr->ai_addr, addr->ai_addrlen) < 0) {
			close(ret);
			eerror("Failed to bind socket");
			continue;

		} else if (listen(ret, SOMAXCONN) < 0) {
			close(ret);
			eerror("Failed to listen");
			continue;
		}

		info("Listening on %s:%s", address ? address : "*", service);
		freeaddrinfo(addrs);
		return ret;
	}

	error("No suitable network address found");
	freeaddrinfo(addrs);
	return -1;
}

int get_listener(const char *interface, const char *service)
{
	socklen_t salen = sizeof(struct sockaddr_in);
	struct ifaddrs *ifas, *ifa;
	char address[NI_MAXHOST];
	int ret;

	if (strcmp(interface, WILDCARD_INTERFACE) == 0) {
		return get_addr_listener(NULL, service);

	} else if (getifaddrs(&ifas) < 0) {
		eerror("Failed to get interfaces info");
		return -1;
	}

	for (ifa = ifas; ifa != NULL; ifa = ifa->ifa_next) {
		if (strcmp(interface, ifa->ifa_name) != 0) {
			continue;

		} else if (ifa->ifa_addr == NULL) {
			continue;

		} else if (ifa->ifa_addr->sa_family != AF_INET) {
			continue;

		} else if ((ret = getnameinfo(ifa->ifa_addr, salen,
		                              address, NI_MAXHOST, NULL, 0,
		                              NI_NUMERICHOST)) < 0) {
			error("Failed to get interface "
			      "address: %s", gai_strerror(ret));
			continue;

		} else if ((ret = get_addr_listener(address, service)) < 0) {
			error("Failed to get listener "
			      "from address: %s", address);
			continue;
		}

		freeifaddrs(ifas);
		return ret;
	}

	error("No suitable interface found");
	freeifaddrs(ifas);
	return -1;
}
