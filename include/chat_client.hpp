#ifndef CHAT_CHAT_CLIENT_HPP
#define CHAT_CHAT_CLIENT_HPP

#include <string>

#include "net_utils.hpp"
#include "protocol_buffer.hpp"

namespace chat {

class ChatClient {
private:
	bool connected = false;

	net::ScopedFileDescriptor client_fd {};
	net::ScopedFileDescriptor signalfd {};

	net::SocketAddress address {};

	static constexpr unsigned int max_name_len = 32;
	std::string client_name = "No Name";

	protocol::Buffer read_buffer {};
	protocol::Buffer write_buffer {};

	void setup_signal_fd();

public:
	ChatClient() = default;
	explicit ChatClient(const std::string &ip_address);
	ChatClient(const std::string &ip_address, const std::string &name);

	ChatClient(const ChatClient&) = delete;
	ChatClient& operator=(const ChatClient&) = delete;
	ChatClient(ChatClient&&) = delete;
	ChatClient& operator=(ChatClient&&) = delete;

	void set_name(const std::string &name);

	void run();
	void stop();

	void event_loop();

	// get client information
	bool is_connected() const noexcept;
	const std::string get_name() const;

	~ChatClient() = default;
};

} // namespace

#endif
