#ifndef CHAT_CHAT_SERVER_HPP
#define CHAT_CHAT_SERVER_HPP

#include <string>
#include <vector>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net_utils.hpp"
#include "protocol_buffer.hpp"

namespace chat {

class ChatServer {
private:
	struct ClientInfo {
		net::ScopedFileDescriptor fd;

		std::string name;

		protocol::Buffer buffer {};
		
		bool connected = false;

		ClientInfo(int fd_, std::string name_):
			fd {fd_}, name {name_}
		{}
	};

	bool running = false;
	net::ScopedFileDescriptor listenfd {};
	net::ScopedFileDescriptor signalfd {}; // for stopping event loop
	std::vector<ClientInfo> clients;

	net::SocketAddress address {};

	int backlog = 64;
	int max_clients = 62; // 64 - 2: one is lsiten fd and another is signalfd

	void setup_signal_fd();
	void respond_to_client(int sender_index);
	void get_client_name(int sender_index);

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
