#include <cerrno>
#include <utility>

#include <fcntl.h>
#include <gtest/gtest.h>

#include "net_utils.hpp"

// class net::SocketFileDescriptor
TEST(NetUtilsTest, ScopedFileDescriptorDefaultConstructor) {
	net::ScopedFileDescriptor sfd {};

	ASSERT_EQ(sfd.get(), -1);
	ASSERT_FALSE(sfd.valid());
}

TEST(NetUtilsTest, ScopedFileDescriptorReset) {
	net::ScopedFileDescriptor sfd {4};

	ASSERT_TRUE(sfd.valid());

	sfd.reset();
	ASSERT_EQ(sfd.get(), -1);
	ASSERT_FALSE(sfd.valid());
}

TEST(NetUtilsTest, ScopedFileDescriptorMoveConstructor) {
	net::ScopedFileDescriptor sfd1 {4};
	net::ScopedFileDescriptor sfd2 {std::move(sfd1)};

	ASSERT_EQ(sfd2.get(), 4);
	ASSERT_TRUE(sfd2.valid());
}

TEST(NetUtilsTest, ScopedFileDescriptorMoveAssignment) {
	int pipefd[2];
	ASSERT_EQ(::pipe(pipefd), 0);

	int read_fd = pipefd[0];
	int write_fd = pipefd[1];

	net::ScopedFileDescriptor first {read_fd};
	net::ScopedFileDescriptor second {write_fd};

	second = std::move(first);

	// check write_fd is closed
	errno = 0;
	ASSERT_TRUE(
			::fcntl(write_fd, F_GETFD) == -1 && errno == EBADF
	);

	ASSERT_EQ(second.get(), read_fd);
}

// struct net::SocketAddress
TEST(NetUtilsTest, SocketAddressFields) {
	struct net::SocketAddress sa {};

	ASSERT_EQ(sa.family, AF_INET);
	ASSERT_EQ(sa.ip_address, INADDR_ANY);
	ASSERT_EQ(sa.port, 9877); // default port
}
