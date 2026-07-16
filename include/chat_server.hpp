#ifndef CHAT_CHAT_SERVER_HPP
#define CHAT_CHAT_SERVER_HPP

#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_utils.hpp"

namespace chat {

class ChatServer {
private:
	bool running = false;
	net::ScopedFileDescriptor listenfd {};
	net::ScopedFileDescriptor signalfd {}; // for stopping event loop
	std::vector<net::ScopedFileDescriptor> clients;

	net::SocketAddress address {};

	int backlog = 64;
	int max_clients = 62; // 64 - 2: one is lsiten fd and another is signalfd

	void setup_signal_fd();
	void respond_to_client();

public:
	ChatServer();
	explicit ChatServer(unsigned short port);

	ChatServer(const ChatServer&) = delete;
	ChatServer& operator=(const ChatServer&) = delete;
	ChatServer(ChatServer&&) = delete;
	ChatServer& operator=(ChatServer&&) = delete;

	void run();
	void stop();
	void event_loop();

	// get server information
	std::string endpoint() const;
	bool is_running() const noexcept;
	unsigned short get_port() const noexcept;

	~ChatServer() = default;
};

} // namespace

#endif
