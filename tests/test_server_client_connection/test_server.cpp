#include "chat_server.hpp"
#include "net_utils.hpp"

int main() {
	constexpr int port = 9877;
	chat::ChatServer server {port};

	server.run();
	server.event_loop();

	return 0;
}
