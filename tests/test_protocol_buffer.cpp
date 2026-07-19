#include <string>
#include <stdexcept>

#include <unistd.h>
#include <gtest/gtest.h>

#include "protocol_buffer.hpp"

// constructor
TEST(ProtocolBufferTest, ConstructorNormal) {
	protocol::Buffer buffer {};
}

// capacity()
TEST(ProtocolBufferTest, CapacityForBuffer) {
	protocol::Buffer buffer {};

	ASSERT_EQ(protocol::buffer_size, buffer.capacity());
}

// size()
TEST(ProtocolBufferTest, SizeIsZeroByDefault) {
	protocol::Buffer buffer {};

	ASSERT_EQ(buffer.size(), 0);
}

TEST(ProtocolBufferTest, AppendAndCheckSize) {
	protocol::Buffer buffer {};

	std::string str {"Hello, World!"};
	buffer.append(str.data(), str.size());

	ASSERT_EQ(buffer.size(), str.size());
}

TEST(ProtocolBufferTest, SizeIsZeroAfterFullConsume) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0);

	int read_fd = fds[0];
	int write_fd = fds[1];

	protocol::Buffer buffer {};

	std::string str {"Hello, World!\n"};
	buffer.append(str.data(), str.size());

	buffer.consume_full(write_fd);

	ASSERT_EQ(buffer.size(), 0);
}

TEST(ProtocolBufferTest, SizeIsNotZeroAfterPartialConsume) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0);

	int read_fd = fds[0];
	int write_fd = fds[1];

	protocol::Buffer buffer {};

	std::string str {"Hello, World!\nHi"};
	buffer.append(str.data(), str.size());

	buffer.consume_full(write_fd);

	ASSERT_EQ(buffer.size(), 2);
}

// append()
TEST(ProtocolBufferTest, AppendTooManyThrows) {
	protocol::Buffer buffer {};

	std::string str(2000, 'c');

	ASSERT_THROW(
			buffer.append(str.data(), str.size()),
			std::length_error
	);
}

// consume_full()
TEST(ProtocolBufferTest, EmptyBufferConsumesNothing) {
	int fds[2];
	ASSERT_EQ(::pipe(fds), 0);

	int read_fd = fds[0];
	int write_fd = fds[1];

	protocol::Buffer buffer {};

	buffer.consume_full(write_fd);
}

TEST(ProtocolBufferTest, ConsumeOnInvalidFDThrows) {
	protocol::Buffer buffer {};

	std::string str {"Hello, World!\n"};
	buffer.append(str.data(), str.size());

	ASSERT_THROW(

			buffer.consume_full(-1),
			std::system_error
	);
}

// free_space()
TEST(ProtocolBufferTest, FreeSpaceIsZeroAfterAppend) {
	protocol::Buffer buffer {};

	std::string str(protocol::buffer_size, 'c');
	buffer.append(str.data(), str.size());

	ASSERT_EQ(buffer.free_space(), 0);
}

// is_empty()
TEST(ProtocolBufferTest, ConstructedBufferIsEmpty) {
	protocol::Buffer buffer {};

	ASSERT_TRUE(buffer.is_empty());
}

TEST(ProtocolBufferTest, AppendedBufferIsNotEmpty) {
	protocol::Buffer buffer {};

	std::string str(protocol::buffer_size, 'c');
	buffer.append(str.data(), str.size());

	ASSERT_FALSE(buffer.is_empty());
}

// is_full()
TEST(ProtocolBufferTest, ConstructedBufferIsNotFull) {
	protocol::Buffer buffer {};

	ASSERT_FALSE(buffer.is_full());
}

TEST(ProtocolBufferTest, PartialAppendedBufferIsNotFull) {
	protocol::Buffer buffer {};

	std::string str(10, 'c');
	buffer.append(str.data(), str.size());

	ASSERT_FALSE(buffer.is_full());
}

TEST(ProtocolBufferTest, FullAppendedBufferIsNotFull) {
	protocol::Buffer buffer {};

	std::string str(protocol::buffer_size, 'c');
	buffer.append(str.data(), str.size());

	ASSERT_TRUE(buffer.is_full());
}
