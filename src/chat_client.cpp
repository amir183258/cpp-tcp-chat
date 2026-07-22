#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <stdexcept>
#include <system_error>
#include <csignal>
#include <cerrno>

#include <poll.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/signalfd.h>

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

	setup_signal_fd();

	// send name for the server
	std::string name = client_name + "\n";
	net::write_full(temp_socket.get(), name.c_str(), name.size());

	// move to main fd
	client_fd = std::move(temp_socket);

	connected = true;
}

void ChatClient::event_loop() {
	if (!connected)
		throw std::logic_error("client is not running");

	std::vector<struct pollfd> pollfds;
	pollfds.push_back({STDIN_FILENO, POLLIN, 0});
	pollfds.push_back({client_fd.get(), POLLIN, 0});
	pollfds.push_back({signalfd.get(), POLLIN, 0});

	int n;
	int nready; // number of ready poll fds
	std::string server_messages {};
	while(connected) {
		nready = ::poll(pollfds.data(), pollfds.size(), -1);
		if (nready < 0) {
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll failed");
		}

		// data is ready in stdin
		if (pollfds[0].revents & POLLIN) {
			n = net::read_fd(STDIN_FILENO,
					write_buffer.data() + write_buffer.size(),
					write_buffer.free_space());
			// EOF
			if (n == 0) {
				stop();
				break;
			}

			write_buffer.increase_size(n);

			// write stdin input to the server fd
			write_buffer.consume_full(client_fd.get());
		}

		// handling server response
		if (pollfds[1].revents & POLLIN) {
			n = net::read_fd(client_fd.get(),
					read_buffer.data() + read_buffer.size(),
					read_buffer.free_space());
			read_buffer.increase_size(n);

			server_messages = read_buffer.consume_full();

			if (server_messages.size() > 0)
				std::cout << server_messages;
		}

		// handling signals
		if (pollfds[2].revents & POLLIN) {
			struct signalfd_siginfo siginfo {};
			ssize_t n = net::read_fd(signalfd.get(), &siginfo, sizeof(siginfo));
			if (n != sizeof(siginfo))
				throw std::runtime_error("failed to read signalfd");

			stop();
			break; // get out of event loop
		}

	}
}

void ChatClient::stop() {
	signalfd.reset();
	client_fd.reset();

	connected = false;
}

bool ChatClient::is_connected() const noexcept {
	return connected;
}

const std::string ChatClient::get_name() const {
	return client_name;
}

// private functions
void ChatClient::setup_signal_fd() {
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

} // namespace
