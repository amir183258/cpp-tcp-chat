#include <iostream>
#include <string>

#include "chat_client.hpp"

int main() {
	chat::ChatClient client {};

	std::string name;
	std::cout <<"Enter your name: " << std::endl;
	std::cin >> name;

	client.set_name(name);

	client.run();

	client.event_loop();

	return 0;
}
