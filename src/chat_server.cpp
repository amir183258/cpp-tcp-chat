#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <algorithm>
#include <csignal>
#include <cstring>

#include <poll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/signalfd.h>

#include "chat_server.hpp"
#include "net_utils.hpp"

namespace chat {

ChatServer::ChatServer() {
	// 1 for listening fd and 1 for signal fd
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

	// using reuse address option for socket
	int opt = 1;
	if (::setsockopt(temp_socket.get(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

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
	clients.clear();

	signalfd.reset();
	listenfd.reset();

	running = false;
}

void ChatServer::event_loop() {
	if (!running)
		throw std::logic_error("server is not running");

	std::vector<struct pollfd> pollfds;
	pollfds.reserve(static_cast<std::size_t>(max_clients + 2));

	int connfd; // accepted client fd
	char buffer[buffer_size]; // read buffer
	int n; // read size
	int nready; // number of ready poll fds
	short int revents; // poll events
	while (running) {
		pollfds.clear();

		pollfds.push_back({listenfd.get(), POLLIN, 0});
		pollfds.push_back({signalfd.get(), POLLIN, 0});

		for (const auto& client : clients)
			pollfds.push_back({client.fd.get(), POLLIN, 0});

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
				clients.push_back({connfd});
				pollfds.push_back({connfd, POLLIN, 0});
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

			stop();
			break; // get out of event loop
		}

		// handling clients
		for (int i = 0; i < clients.size(); ) {
			// client error in poll
			revents = pollfds[i + 2].revents;
			if ( (revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
				clients[i].fd = std::move(clients.back().fd);
				clients.pop_back();

				pollfds[i + 2] = pollfds.back();
				pollfds.pop_back();

				if (--nready <= 0)
					break;

				continue;
			}

			// client is ready to be read
			if (revents & POLLIN) {
				n = net::read_fd(clients[i].fd.get(),
						clients[i].buffer + clients[i].end,
						sizeof(clients[i].buffer) - clients[i].end);
				if (n < 0) {
					// connection reset by client
					if (errno == ECONNRESET) {
						clients[i].fd = std::move(clients.back().fd);
						clients.pop_back();

						pollfds[i + 2] = pollfds.back();
						pollfds.pop_back();
					}
					else
						throw std::runtime_error("failed to read from client");
				}
				// connection closed by the client
				else if (n == 0) {
					clients[i].fd = std::move(clients.back().fd);
					clients.pop_back();

					pollfds[i + 2] = pollfds.back();
					pollfds.pop_back();
				}
				else {
					clients[i].end += n;

					respond_to_client(i, clients[i].buffer, sizeof(clients[i].buffer));
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
	addr.s_addr = ::htonl(address.ip_address);

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

void ChatServer::respond_to_client(int sender_index, char *buffer, size_t buffer_len) {
	int last_endline_idx = -1;
	int start = clients[sender_index].start;
	int end = clients[sender_index].end - 1;
	while (start <= end) {
		if (buffer[end] == '\n') {
			last_endline_idx = end;
			break;
		}

		--end;
	}

	if (last_endline_idx == -1 && clients[sender_index].end - clients[sender_index].start == buffer_len) {
		clients[sender_index].start = 0;
		clients[sender_index].end= 0;

		return;
	}

	if (last_endline_idx != -1) {
		int send_start = clients[sender_index].start;
		int send_len = last_endline_idx - send_start + 1;

		for (int i = 0; i < clients.size(); i++) {
			// do nothing for the sender client
			if (i == sender_index)
				continue;

			

			net::write_full(clients[i].fd.get(),
					reinterpret_cast<const void *>(buffer + send_start), send_len);
		}
	}

	int old_end = clients[sender_index].end;
	int new_start = (last_endline_idx != -1) ?
		last_endline_idx + 1 : clients[sender_index].start;

	int remaining_len = old_end - new_start;

	if (remaining_len > 0 && new_start > 0)
		memmove(buffer, buffer + new_start, remaining_len);

	clients[sender_index].start = 0;
	clients[sender_index].end = remaining_len;
}

} // namespace
