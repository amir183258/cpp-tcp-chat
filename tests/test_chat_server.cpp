#include <stdexcept>
#include <string>

#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
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

// event_loop()
TEST(ChatServerTest, ShutsDownOnSigint) {
	// create a pipe for child to notify parent
	int ready_pipe[2];
	ASSERT_EQ(::pipe(ready_pipe), 0);

	pid_t pid = ::fork();
	ASSERT_NE(pid, -1);

	// child code
	if (pid == 0) {
		::close(ready_pipe[0]); // child does not read

		try {
			chat::ChatServer server {};
			server.run();

			char ready = '1';

			if (::write(ready_pipe[1], &ready, 1) != 1)
				std::exit(3);

			server.event_loop();

			std::exit(0);
		}
		catch (...) {
			std::exit(2);
		}
	}

	// parent again
	::close(ready_pipe[1]); // parent does not write

	char ready = 0;
	ASSERT_EQ(::read(ready_pipe[0], &ready, 1), 1);
	ASSERT_EQ(ready, '1');

	ASSERT_EQ(::kill(pid, SIGINT), 0);

	int status = 0;
	ASSERT_EQ(::waitpid(pid, &status, 0), pid);

	ASSERT_TRUE(WIFEXITED(status));
	ASSERT_EQ(WEXITSTATUS(status), 0);
}

TEST(ChatServerTest, ClientConnects) {
	int ready_pipe[2];
	ASSERT_EQ(::pipe(ready_pipe), 0);

	pid_t pid = ::fork();
	ASSERT_NE(pid, -1);

	// child code
	if (pid == 0) {
		::close(ready_pipe[0]); // child does not read

		try {
			chat::ChatServer server {};
			server.run();

			char ready = '1';

			if (::write(ready_pipe[1], &ready, 1) != 1)
				std::exit(3);

			server.event_loop();

			std::exit(0);
		}
		catch (...) {
			std::exit(2);
		}
	}

	// parent again
	::close(ready_pipe[1]); // parent does not write

	char ready = 0;
	ASSERT_EQ(::read(ready_pipe[0], &ready, 1), 1);
	ASSERT_EQ(ready, '1');

	// client connects
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(fd, 0);

	struct sockaddr_in servaddr {};
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	ASSERT_GE(::inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr), 0);

	ASSERT_EQ(::connect(fd, reinterpret_cast<struct sockaddr*>(&servaddr),
				sizeof(servaddr)), 0);

	ASSERT_EQ(::shutdown(fd, SHUT_WR), 0);

	ASSERT_EQ(::kill(pid, SIGINT), 0);

	int status = 0;
	ASSERT_EQ(::waitpid(pid, &status, 0), pid);

	ASSERT_TRUE(WIFEXITED(status));
	ASSERT_EQ(WEXITSTATUS(status), 0);
}

TEST(ChatServerTest, MoreThanMaxClientsConnectToServer) {
	int ready_pipe[2];
	ASSERT_EQ(::pipe(ready_pipe), 0);

	pid_t pid = ::fork();
	ASSERT_NE(pid, -1);

	// child code
	if (pid == 0) {
		::close(ready_pipe[0]); // child does not read

		try {
			chat::ChatServer server {};
			server.run();

			char ready = '1';

			if (::write(ready_pipe[1], &ready, 1) != 1)
				std::exit(3);

			server.event_loop();

			std::exit(0);
		}
		catch (...) {
			std::exit(2);
		}
	}

	// parent again
	::close(ready_pipe[1]); // parent does not write

	char ready = 0;
	ASSERT_EQ(::read(ready_pipe[0], &ready, 1), 1);
	ASSERT_EQ(ready, '1');

	struct sockaddr_in servaddr {};
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	ASSERT_GE(::inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr), 0);

	// clients connects
	constexpr int max_clients = 62;
	int fds[max_clients];
	for (int i = 0; i < max_clients; i++) {
		fds[i] =  ::socket(AF_INET, SOCK_STREAM, 0);
		ASSERT_GE(fds[i], 0);

		ASSERT_EQ(::connect(fds[i], reinterpret_cast<struct sockaddr*>(&servaddr),
					sizeof(servaddr)), 0);
	}

	// more clients than max_client have to throw
	// I used poll here
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(fd, 0);

	ASSERT_EQ(::connect(fd, reinterpret_cast<struct sockaddr*>(&servaddr),
				sizeof(servaddr)), 0);

	// wait for server to close the client
	pollfd pfd {};
	pfd.fd = fd;
	pfd.events = POLLIN | POLLHUP;

	int poll_result = ::poll(&pfd, 1, 2000);
	ASSERT_GT(poll_result, 0);
	ASSERT_TRUE((pfd.revents & POLLIN) != 0 || (pfd.revents & POLLHUP) != 0);

	char byte {};
	ASSERT_EQ(::read(fd, &byte, 1), 0); // this shows server has closed the extra client

	ASSERT_EQ(::close(fd), 0);

	// clean up sockets
	for (int i = 0; i < max_clients; i++)
		ASSERT_EQ(::close(fds[i]), 0);

	ASSERT_EQ(::kill(pid, SIGINT), 0);

	int status = 0;
	ASSERT_EQ(::waitpid(pid, &status, 0), pid);

	ASSERT_TRUE(WIFEXITED(status));
	ASSERT_EQ(WEXITSTATUS(status), 0);
}

// respond_to_client()
TEST(ChatServerTest, ServerRespondToCLientNormal) {
	// create server in a child process
	int ready_pipe[2];
	ASSERT_EQ(::pipe(ready_pipe), 0);

	pid_t pid = ::fork();
	ASSERT_NE(pid, -1);

	// child code
	if (pid == 0) {
		::close(ready_pipe[0]); // child does not read

		try {
			chat::ChatServer server {};
			server.run();

			char ready = '1';

			if (::write(ready_pipe[1], &ready, 1) != 1)
				std::exit(3);

			server.event_loop();

			std::exit(0);
		}
		catch (...) {
			std::exit(2);
		}
	}

	// parent again
	::close(ready_pipe[1]); // parent does not write

	char ready = 0;
	ASSERT_EQ(::read(ready_pipe[0], &ready, 1), 1);
	ASSERT_EQ(ready, '1');

	struct sockaddr_in servaddr {};
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(9877);
	ASSERT_GE(::inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr), 0);

	// clients connects
	int client_fd1;
	int client_fd2;

	client_fd1 = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(client_fd1, 0);
	ASSERT_EQ(::connect(client_fd1, reinterpret_cast<struct sockaddr*>(&servaddr),
				sizeof(servaddr)), 0);

	client_fd2 = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT_GE(client_fd2, 0);
	ASSERT_EQ(::connect(client_fd2, reinterpret_cast<struct sockaddr*>(&servaddr),
				sizeof(servaddr)), 0);

	// write to server
	std::string message {"Hello, World!\n"};
	ASSERT_GE(::write(client_fd1, message.data(), message.size()), 0);

	// read response
	char buffer[1024];
	ASSERT_GE(::read(client_fd2, &buffer, sizeof(buffer)), 0);

	ASSERT_EQ(strcmp(message.data(), buffer), 0);

	ASSERT_EQ(::close(client_fd1), 0);
	ASSERT_EQ(::close(client_fd2), 0);

	// kill child process
	ASSERT_EQ(::kill(pid, SIGINT), 0);

	int status = 0;
	ASSERT_EQ(::waitpid(pid, &status, 0), pid);

	ASSERT_TRUE(WIFEXITED(status));
	ASSERT_EQ(WEXITSTATUS(status), 0);
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
