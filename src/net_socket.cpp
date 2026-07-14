#include <system_error>
#include <cerrno>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_utils.hpp"

namespace net {

int accept_socket(int fd, struct sockaddr *sa, socklen_t *salenptr) {
	int n;
	for (;;) {
		n = accept(fd, sa, salenptr);
		if (n >= 0)
			return n;

#ifdef ERPROTO
		if (errno == ERPROTO || errno == ECONNABORTED)
			continue;
#else
		if (errno == ECONNABORTED)
			continue;
#endif

		throw std::system_error(errno, std::generic_category(), "accept() failed");
	}
}

int create_socket(int family, int type, int protocol) {
	int fd = ::socket(family, type, protocol);
	if (fd < 0)
		throw std::system_error(errno, std::generic_category(), "socket() failed");
	return fd;
}

void connect_socket(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (::connect(fd, sa, salen) < 0)
		throw std::system_error(errno, std::generic_category(), "connect() failed");
}

void bind_socket(int fd, const struct sockaddr *sa, socklen_t salen) {
	if (::bind(fd, sa, salen) < 0)
		throw std::system_error(errno, std::generic_category(), "bind() failed");
}

void listen_socket(int fd, int backlog) {
	if (::listen(fd, backlog) < 0)
		throw std::system_error(errno, std::generic_category(), "listen() failed");
}

} // namespace
