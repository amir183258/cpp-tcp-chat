#include <string>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <unistd.h>
#include <gtest/gtest.h>

#include "net_utils.hpp"

using ScopedFd = net::ScopedFileDescriptor;

// close_fd()
TEST(NetUtilsTest, CloseFileDescriptorNormal) {
	int fd = -1;
	fd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(fd, 0);

	ScopedFd socket_fd {fd};
}

TEST(NetUtilsTest, CloseFileDescriptorThrows) {
	ASSERT_THROW(
			net::close_fd(-1),
			std::system_error
	);
}

// read_fd()
TEST(NetUtilsTest, ReadFileDescriptorNormal) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0)
		<< "pipe() failed: " << std::strerror(errno);

	ScopedFd first_fd {fds[0]};
	ScopedFd second_fd {fds[1]};

	constexpr std::string_view message = "hello";
	ASSERT_EQ(::write(fds[1], message.data(), message.size()),
			static_cast<ssize_t>(message.size()))
		<< "write() failed: " << std::strerror(errno);

	char buffer[32] {};

	ssize_t nread = -1;
	ASSERT_NO_THROW (
			nread = net::read_fd(fds[0], buffer, sizeof(buffer));
	);

	ASSERT_EQ(nread, static_cast<ssize_t>(message.size()));
	ASSERT_EQ(std::string_view(buffer, static_cast<std::size_t>(nread)),
			message);
}

TEST(NetUtilsTest, ReadFileDescriptorReturnsZeroAtEndOfFile) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0)
		<< "pipe() failed: " << std::strerror(errno);

	ScopedFd first_fd {fds[0]};
	::close(fds[1]);

	char buffer[32] {};
	ssize_t nread = -1;

	ASSERT_NO_THROW(
			nread = net::read_fd(fds[0], buffer, sizeof(buffer));
	);

	ASSERT_EQ(nread, 0);
}

TEST(NetUtilsTest, ReadFileDescriptorThrowsForInvalidDescriptor) {
	char buffer[32];

	ASSERT_THROW(
			net::read_fd(-1, buffer, sizeof(buffer)),
			std::system_error
	);
}

// write_fd()
TEST(NetUtilsTest, WriteFileDescriptorNormal) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0)
		<< "pipe() failed: " << std::strerror(errno);

	ScopedFd second_fd {fds[1]};
	::close(fds[0]);

	char buffer[] = "Hello";
	ASSERT_NO_THROW(
			net::write_fd(fds[1], buffer, sizeof(buffer));
	);
}

TEST(NetUtilsTest, WriteFileDescriptorThrows) {
	ASSERT_THROW(
			net::write_fd(-1, nullptr, 0), 
			std::system_error
	);
}
