#include <system_error>

#include <gtest/gtest.h>

#include "chat_client.hpp"
#include "chat_server.hpp"

// constructor
TEST(ChatClientTest, DefaultConstructor) {
	chat::ChatClient client {};

	ASSERT_FALSE(client.is_connected());
}

TEST(ChatClientTest, ConstructUsingIpAddress) {
	chat::ChatClient client {"127.0.0.1"};

	ASSERT_FALSE(client.is_connected());
}

TEST(ChatClientTest, ConstructUsingIpAddressAndName) {
	chat::ChatClient client {"127.0.0.1", "Amir"};

	ASSERT_FALSE(client.is_connected());
}

TEST(ChatClientTest, ConstructThrowsForBadIp) {
	ASSERT_THROW(

			chat::ChatClient client {"127.260.0.1"};,
			std::system_error
	);
}

// set_name()
TEST(ChatClientTest, SetNameAfterCreatingClient) {
	chat::ChatClient client {};

	client.set_name("Amir");

	ASSERT_EQ(client.get_name(), "Amir");
}

// run()
TEST(ChatClientTest, RunClientNormal) {
	// server
	chat::ChatServer server {}; server.run();

	// client
	chat::ChatClient client {};
	client.run();

	// cleanup is done using RAII in client class
}

TEST(ChatClientTest, RunClientTwiceThrows) {
	// server
	chat::ChatServer server {};
	server.run();

	// client
	chat::ChatClient client {};
	client.run();

	ASSERT_THROW(
			client.run(),
			std::logic_error
	);

	// cleanup is done using RAII in client class
}

// stop()
TEST(ChatClientTest, RunClientAndStop) {
	// server
	chat::ChatServer server {};
	server.run();

	// client
	chat::ChatClient client {};
	client.run();

	client.stop();

	ASSERT_FALSE(client.is_connected());

	// cleanup is done using RAII in client class
}
