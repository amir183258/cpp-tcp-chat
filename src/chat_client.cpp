#include <string>
#include <utility>
#include <stdexcept>

#include "chat_client.hpp"
#include "net_utils.hpp"

namespace chat {

ChatClient::ChatClient(const std::string &ip_address) {
	net::inet_pton_socket(AF_INET, ip_address.c_str(), &address.ip_address);
}

ChatClient::ChatClient(const std::string & ip_address, const std::string &name) 
	: ChatClient(ip_address)

{
	set_name(name);
}

void ChatClient::set_name(const std::string &name) {
	if (name.empty())
		throw std::invalid_argument("client name cannot be empty");

	if (name.size() > max_name_len)
		throw std::length_error("client name exceeds maximum length");

	client_name = name;
}

void ChatClient::run() {
	if (connected)
		throw std::logic_error("client is already connected");

	struct sockaddr_in servaddr = address.to_sockaddr();

	// create temporary socket
	// cleanup is easy using RAII
	net::ScopedFileDescriptor temp_socket {
		net::create_socket(AF_INET, SOCK_STREAM, 0)
	};

	// connect to server
	net::connect_socket(temp_socket.get(),
			reinterpret_cast<sockaddr*>(&servaddr),
			sizeof(servaddr));

	// move to main fd
	client_fd = std::move(temp_socket);
	connected = true;
}

void ChatClient::stop() {
	client_fd.reset();

	connected = false;
}

bool ChatClient::is_connected() const noexcept {
	return connected;
}

const std::string ChatClient::get_name() const {
	return client_name;
}

} // namespace
