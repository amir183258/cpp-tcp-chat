#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <gtest/gtest.h>

#include "net_utils.hpp"

// scoped file descriptor: using RAII for socket file descriptors
class ScopedFd {
private:
	int fd_;

public:
	explicit ScopedFd(int fd = -1): fd_ {fd} {}

	ScopedFd(const ScopedFd&) = delete;
	ScopedFd& operator=(const ScopedFd&) = delete;

	int get() const {
		return fd_;
	}

	~ScopedFd() {
		if (fd_ >= 0)
			::close(fd_);
	}
};

// create_socket()
TEST(NetUtilsTest, CreateSocketNormal) {
	int fd = -1;

	ASSERT_NO_THROW (
			fd = net::create_socket(AF_INET, SOCK_STREAM, 0);
	);
	EXPECT_GE(fd, 0);

	::close(fd);
}

TEST(NetUtilsTest, CreateSocketThrowsForInvalidFamily) {
	EXPECT_THROW(
			net::create_socket(-1, SOCK_STREAM, 0),
			std::system_error
	);
}

// connect_socket()
TEST(NetUtilsTest, ConnectSocketNormal) {
	int listenfd = -1;
	ASSERT_NO_THROW(
			listenfd = net::create_socket(AF_INET, SOCK_STREAM, 0);
	);
	ScopedFd listener {listenfd};

	int connfd = -1;
	ASSERT_NO_THROW(
			connfd = net::create_socket(AF_INET, SOCK_STREAM, 0);
	);
	ScopedFd client {connfd};

	// server listener
	sockaddr_in servaddr {};
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	servaddr.sin_port = htons(0);

	ASSERT_EQ(
			::bind(listener.get(),
				reinterpret_cast<sockaddr*>(&servaddr),
				sizeof(servaddr)),
			0) << "bind() failed: " << std::strerror(errno);

	ASSERT_EQ(
			::listen(listener.get(), 16),
			0) << "listen() failed: " << std::strerror(errno);

	socklen_t addrlen = sizeof(servaddr);
	ASSERT_EQ(
			::getsockname(listener.get(),
				reinterpret_cast<sockaddr*>(&servaddr),
				&addrlen),
			0) << "getsockname() failed: " << std::strerror(errno);

	// client
	ASSERT_NO_THROW(
			net::connect_socket(client.get(),
				reinterpret_cast<sockaddr*>(&servaddr),
				sizeof(servaddr)));
}
}
