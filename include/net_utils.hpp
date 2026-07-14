#ifndef NET_NET_UTILS_HPP
#define NET_NET_UTILS_HPP

#include<sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace net {

struct SocketAddress {
	int family = AF_INET;
	unsigned long int ip_address = INADDR_ANY;
	unsigned short port = 9877;

	sockaddr_in to_sockaddr() const {
		sockaddr_in addr {};
		addr.sin_family = family;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(ip_address);

		return addr;
	}
};

// net_socket.cpp
int accept_socket(int fd, struct sockaddr *sa, socklen_t *salenptr);
int create_socket(int family, int type, int protocol = 0);
void connect_socket(int fd, const struct sockaddr *sa, socklen_t salen);
void bind_socket(int fd, const struct sockaddr *sa, socklen_t salen);
void listen_socket(int fd, int backlog = SOMAXCONN);

// net_unix.cpp
void close_fd(int fd);
ssize_t read_fd(int fd, void *ptr, size_t nbytes);
void write_fd(int fd, void *ptr, size_t nbytes);

} // namespace

#endif
