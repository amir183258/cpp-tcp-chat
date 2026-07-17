#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <gtest/gtest.h>

#include "net_utils.hpp"

using ScopedFd = net::ScopedFileDescriptor;

// accept_socket()
TEST(NetUtilsTest, AcceptSocketNormal) {
	int listenfd = -1;
	listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(listenfd, 0);

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
	ASSERT_EQ(
			::connect(client.get(),
				reinterpret_cast<sockaddr*>(&servaddr),
				sizeof(servaddr)),
			0) << "connect() failed: " << std::strerror(errno);

	// accept
	int accfd = -1;
	ASSERT_NO_THROW(
			accfd = net::accept_socket(listenfd, nullptr, nullptr)
	);
	ScopedFd accepted {accfd};
}

TEST(NetUtilsTest, AcceptSocketThrows) {
	EXPECT_THROW(
			net::accept_socket(-1, nullptr, nullptr),
			std::system_error
	);
}

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
	listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(listenfd, 0);

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

TEST(NetUtilsTest, ConnectSocketThrows) {
	ASSERT_THROW(
			net::connect_socket(-1, nullptr, 0),
			std::system_error
	);
}

// bind_socket()
TEST(NetUtilsTest, BindSocketNormal) {
	int listenfd = -1;
	listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(listenfd, 0);

	ScopedFd listener {listenfd};

	// server listener
	sockaddr_in servaddr {};
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	servaddr.sin_port = htons(0);

	ASSERT_NO_THROW(
			net::bind_socket(listener.get(),
				reinterpret_cast<sockaddr*>(&servaddr),
				sizeof(servaddr))
	);
}

TEST(NetUtilsTest, BindSocketThrows) {
	ASSERT_THROW(
			net::bind_socket(-1, nullptr, 0),
			std::system_error
	);
}

// listen_socket()
TEST(NetUtilsTest, ListenSocketNormal) {
	int listenfd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(listenfd, 0);

	ScopedFd listener {listenfd};

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

	ASSERT_NO_THROW(
			net::listen_socket(listener.get(), 16)
	);
}

TEST(NetUtilsTest, ListenSocketThrows) {
	ASSERT_THROW(
			net::listen_socket(-1, 16),
			std::system_error
	);
}

// poll_socket()
TEST(NetUtilsTest, PollSocketNormal) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0)
		<< "pipe() failed: " << std::strerror(errno);
	ScopedFd first_fd {fds[0]};
	ScopedFd second_ds {fds[1]};

	struct pollfd pfd {};
	pfd.fd = first_fd.get();
	pfd.events = POLLIN;

	int nready = net::poll_socket(&pfd, 1, 0);

	ASSERT_EQ(nready, 0);
	ASSERT_EQ(pfd.revents, 0);
}

TEST(NetUtilsTest, PollSocketThrows) {
	ASSERT_THROW(
			int nready = net::poll_socket(nullptr, 1, 0),
			std::system_error
	);
}

// inet_pton_socket()
TEST(NetUtilsTest, InetPtonSocketNormal) {
	std::string ipv4_address {"192.168.1.2"};

	struct sockaddr_in result_addr {};
	
	ASSERT_NO_THROW(
			net::inet_pton_socket(AF_INET, ipv4_address.c_str(),
				reinterpret_cast<struct sockaddr*>(&result_addr));
	);
}

TEST(NetUtilsTest, InetPtonSocketThrows) {
	std::string ipv4_address {"Hello, World!"};

	struct sockaddr_in result_addr {};
	
	ASSERT_THROW(
			net::inet_pton_socket(AF_INET, ipv4_address.c_str(),
				reinterpret_cast<struct sockaddr*>(&result_addr)),
			std::system_error
	);
}
