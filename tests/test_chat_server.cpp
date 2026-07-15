#include <stdexcept>

#include <gtest/gtest.h>

#include "chat_server.hpp"

// constructor
TEST(ChatServerTest, DefaultConstructor) {
	chat::ChatServer server {};

	EXPECT_FALSE(server.is_running());
	EXPECT_EQ(server.get_port(), 9877);
}

TEST(ChatServerTest, PortConstructor) {
	chat::ChatServer server {9000};

	EXPECT_FALSE(server.is_running());
	EXPECT_EQ(server.get_port(), 9000);
}

// endpoint()
TEST(ChatServerTest, EndpointString) {
	chat::ChatServer server {9000};

	EXPECT_FALSE(server.is_running());
	EXPECT_EQ(server.get_port(), 9000);

	EXPECT_EQ(server.endpoint(), "0.0.0.0:9000");
}

// run()
TEST(ChatServerTest, RunNormal) {
	chat::ChatServer server {};
server.run();
}

TEST(ChatServerTest, RunTwiceReturnThrows) {
	chat::ChatServer server {};

	server.run();

	ASSERT_THROW(
			server.run(),
			std::logic_error
	);
}

// stop()
TEST(ChatServerTest, StopRunningServer) {
	chat::ChatServer server {};

	server.run();
	ASSERT_TRUE(server.is_running());

	server.stop();
	ASSERT_FALSE(server.is_running());
}

TEST(ChatServerTest, StopNotRunningServer) {
	chat::ChatServer server {};
	ASSERT_FALSE(server.is_running());

	server.stop();
	ASSERT_FALSE(server.is_running());
}
