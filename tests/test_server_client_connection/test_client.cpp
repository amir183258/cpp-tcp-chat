#include <iostream>
#include <string>

#include <sys/socket.h>

#include "net_utils.hpp"

int main() {
	int fd = net::create_socket(AF_INET, SOCK_STREAM, 0);
	net::ScopedFileDescriptor sfd {fd};

	net::SocketAddress sa {};
	struct sockaddr_in servaddr = sa.to_sockaddr();

	// connect
	net::connect_socket(sfd.get(), reinterpret_cast<struct sockaddr*>(&servaddr),
			sizeof(servaddr));

	net::close_fd(sfd.get());

	std::cout << "Client is done!" << std::endl;

	return 0;
}
