#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <algorithm>
#include <csignal>

#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/signalfd.h>

#include "chat_server.hpp"
#include "net_utils.hpp"

namespace chat {

ChatServer::ChatServer() {
	clients.reserve(static_cast<std::size_t>(max_clients + 2));
}

ChatServer::ChatServer(unsigned short port)
	: ChatServer()
{

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

	// setup signal descriptor for handling signals
	setup_signal_fd();

	listenfd = std::move(temp_socket);
	running = true;
}

void ChatServer::stop() {
	listenfd.reset();
	signalfd.reset();
	clients.clear();

	running = false;
}

void ChatServer::event_loop() {
	if (!running)
		throw std::logic_error("server is not running");

	std::vector<struct pollfd> pollfds;
	pollfds.reserve(static_cast<std::size_t>(max_clients + 2));

	int connfd; // accepted client fd
	char buffer[1024]; // read buffer
	int n; // read size
	int nready; // number of ready poll fds
	short int revents; // poll events
	while (running) {
		pollfds.clear();

		pollfds.push_back({listenfd.get(), POLLIN, 0});
		pollfds.push_back({signalfd.get(), POLLIN, 0});

		for (const auto& client : clients)
			pollfds.push_back({client.get(), POLLIN, 0});

		nready = ::poll(pollfds.data(), pollfds.size(), -1);
		if (nready < 0) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll failed");
		}
		
		// new client connection
		if (pollfds[0].revents & POLLIN) {
			connfd = net::accept_socket(listenfd.get(), nullptr, nullptr);

			if (clients.size() >= max_clients)
				net::close_fd(connfd);
			else {
				clients.push_back(net::ScopedFileDescriptor(connfd));
				pollfds.push_back({connfd, POLLIN, 0});

				std::cout << "Client added to the server!" << std::endl;
			}

			if (--nready <= 0)
				continue; // no more readable descriptors
		}

		// handling signals
		if (pollfds[1].revents & POLLIN) {
			struct signalfd_siginfo siginfo {};
			ssize_t n = net::read_fd(signalfd.get(), &siginfo, sizeof(siginfo));
			if (n != sizeof(siginfo))
				throw std::runtime_error("failed to read signalfd");

			std::cout << "Signal is here now" << std::endl;

			stop();
			break; // get out of event loop
		}

		// handling clients
		for (int i = 0; i < clients.size(); ) {
			// client error in poll
			revents = pollfds[i + 2].revents;
			if ( (revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
				clients[i] = std::move(clients.back());
				clients.pop_back();

				pollfds[i + 2] = pollfds.back();
				pollfds.pop_back();

				if (--nready <= 0)
					break;

				continue;
			}

			// client is ready to be read
			if (revents & POLLIN) {
				n = net::read_fd(clients[i].get(), buffer, sizeof(buffer));
				if (n < 0) {
					// connection reset by client
					if (errno == ECONNRESET) {
						clients[i] = std::move(clients.back());
						clients.pop_back();

						pollfds[i + 2] = pollfds.back();
						pollfds.pop_back();
					}
					else
						throw std::runtime_error("failed to read from client");
				}
				// connection closed by the client
				else if (n == 0) {
					clients[i] = std::move(clients.back());
					clients.pop_back();

					pollfds[i + 2] = pollfds.back();
					pollfds.pop_back();

					std::cout << "Client discounnected normally!" << std::endl;
				}
				else {
					respond_to_client();
					i++;
				}

				if (--nready <= 0)
					break; // no more readable desciptors
			}
			else
				i++;
		}
	}
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

// private functions
void ChatServer::setup_signal_fd() {
	sigset_t mask {};
	::sigemptyset(&mask);
	::sigaddset(&mask, SIGINT);
	::sigaddset(&mask, SIGTERM);

	// block signals to prevent OS default routine
	if (::sigprocmask(SIG_BLOCK, &mask, nullptr) == -1)
		throw std::system_error(errno, std::generic_category(),
				"failed to block signals using sigprocmask");

	// create the signal file descriptor
	int fd = ::signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
	if (fd == -1)
		throw std::system_error(errno, std::generic_category(),
				"failed to create signalfd");

	signalfd = net::ScopedFileDescriptor {fd};
}

// TODO respond to client
void ChatServer::respond_to_client() {

}

} // namespace
