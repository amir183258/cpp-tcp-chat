#ifndef CHAT_CHAT_SERVER_HPP
#define CHAT_CHAT_SERVER_HPP

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_utils.hpp"

namespace chat {

class ChatServer {
private:
	bool running = false;
	int fd = -1;

	net::SocketAddress address;

public:
	ChatServer() = default;
	explicit ChatServer(unsigned short port);

	void run();

	// get server information
	std::string endpoint() const;
	bool is_running() const noexcept;
	unsigned short get_port() const noexcept;
};

} // namespace

#endif
