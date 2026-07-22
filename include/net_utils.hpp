#ifndef NET_NET_UTILS_HPP
#define NET_NET_UTILS_HPP

#include <string>
#include <utility>

#include<sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

namespace net {

class ScopedFileDescriptor {
private:
	int fd_;

public:
	explicit ScopedFileDescriptor(int fd = -1) noexcept
		: fd_ {fd} {}

	ScopedFileDescriptor(ScopedFileDescriptor &&other) noexcept
		: fd_ {std::exchange(other.fd_, -1)} {}

	ScopedFileDescriptor& operator=(ScopedFileDescriptor &&other) noexcept {
		if (this != &other) {
			reset();
			fd_ = std::exchange(other.fd_, -1);
		}

		return *this;
	}

	ScopedFileDescriptor(const ScopedFileDescriptor&) = delete;
	ScopedFileDescriptor& operator=(const ScopedFileDescriptor&) = delete;

	int get() const {
		return fd_;
	}

	bool valid() const noexcept {
		return fd_ >= 0;
	}

	void reset(int fd = -1) noexcept {
		if (fd_ >= 0)
			::close(fd_);

		fd_ = fd;
	}

	int release() noexcept {
		return std::exchange(fd_, -1);
	}

	~ScopedFileDescriptor() {
		reset();
	}
};

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
int poll_socket(struct pollfd *fdarray, unsigned long nfds, int timeout);
void inet_pton_socket(int family, const char *strptr, void *addrptr);
bool is_valid_ip4(const std::string& ip);

// net_unix.cpp
void close_fd(int fd);
ssize_t read_fd(int fd, void *ptr, size_t nbytes);
void write_fd(int fd, void *ptr, size_t nbytes);

// net_io.cpp
void write_full(int fd, const void *ptr, size_t nbytes);
ssize_t read_full(int fd, void *ptr, size_t nbytes);

} // namespace

#endif
