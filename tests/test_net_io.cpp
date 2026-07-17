#include <string>
#include <cerrno>
#include <cstring>

#include <unistd.h>
#include <gtest/gtest.h>

#include "net_utils.hpp"

using ScopedFd = net::ScopedFileDescriptor;

// write_full()
TEST(NetUtilsTest, WriteFullIntoAFileDescriptorNormal) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0)
		<< "pipe() failed: " << std::strerror(errno);

	ScopedFd read_fd {fds[0]};
	ScopedFd write_fd {fds[1]};

	std::string str = "Hello, World!";
	ASSERT_NO_THROW(
			net::write_full(write_fd.get(), str.data(), str.size())
	);

	char read_buffer[32];
	ASSERT_EQ(
			::read(read_fd.get(), &read_buffer, str.size()),
			str.size()
	);
	read_buffer[str.size()] = '\0';

	ASSERT_EQ(std::string(read_buffer), str);
}

TEST(NetUtilsTest, WriteFullThrowsOnBadFileDescriptor) {
	ASSERT_THROW(
			net::write_full(-1, nullptr, 2),
			std::system_error
	);
}
