#include <string>
#include <stdexcept>
#include <system_error>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "chat_server.hpp"
#include "net_utils.hpp"

namespace chat {

ChatServer::ChatServer(unsigned short port) {
	address.port = port;
}

void ChatServer::run() {
	int listenfd = net::create_socket(address.family, SOCK_STREAM, 0);

	struct sockaddr_in servaddr = address.to_sockaddr();
	net::bind_socket(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

	net::listen_socket(listenfd);
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
