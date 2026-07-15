#include <string>
#include <stdexcept>
#include <system_error>
#include <utility>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "chat_server.hpp"
#include "net_utils.hpp"

namespace chat {

ChatServer::ChatServer(unsigned short port) {
	address.port = port;
}

void ChatServer::run() {
	if (running)
		throw std::logic_error("server is already running");

	struct sockaddr_in servaddr = address.to_sockaddr();

	// create temporary socket
	// cleanup is easy using RAII
	net::ScopedFileDescriptor temp_socket {
		net::create_socket(AF_INET, SOCK_STREAM, 0)
	};

	// bind
	net::bind_socket(temp_socket.get(),
			reinterpret_cast<sockaddr*>(&servaddr),
			sizeof(servaddr));

	// start listening
	net::listen_socket(temp_socket.get(), backlog);

	listenfd = std::move(temp_socket);
	running = true;
}

void ChatServer::stop() {
	listenfd.reset();
	running = false;
}

// get server information
std::string ChatServer::endpoint() const {
	in_addr addr {};
	addr.s_addr = htonl(address.ip_address);

	char buf[INET_ADDRSTRLEN] {};
	if(inet_ntop(address.family, &addr, buf, sizeof(buf)) == nullptr)
		throw std::runtime_error("int_ntop() failed");

	return std::string(buf) + ":" + std::to_string(address.port);
}

bool ChatServer::is_running() const noexcept {
	return running;
}

unsigned short ChatServer::get_port() const noexcept {
	return address.port;
}

} // namespace
