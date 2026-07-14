#include <gtest/gtest.h>

#include "chat_server.hpp"

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

TEST(ChatServerTest, EndpointString) {
	chat::ChatServer server {9000};

	EXPECT_FALSE(server.is_running());
	EXPECT_EQ(server.get_port(), 9000);

	EXPECT_EQ(server.endpoint(), "0.0.0.0:9000");
}
