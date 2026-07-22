#include <string>

#include "net_utils.hpp"
#include "chat_server.hpp"
#include "chat_client.hpp"
#include "CLI11.hpp"

int main(int argc, char **argv) {
	CLI::App app("tcpchat commandline tool");

	app.require_subcommand(1);

	auto run = app.add_subcommand("run", "Run server or client");
	auto server = run->add_subcommand("server", "Run server");
	auto client = run->add_subcommand("client", "Run client");

	short unsigned int server_port = 9877;
	server->add_option("-p,--port", server_port, "Port to listen on");

	std::string client_host = "127.0.0.1";
	client->add_option("--host", client_host, "Server host");

	if (argc == 1) {
		std::cout << app.help() << std::endl;
		return 0;
	}

	CLI11_PARSE(app, argc, argv);

	if (*server) {
		chat::ChatServer srv {server_port};
		srv.run();
		srv.event_loop();

		return 0;
	}

	if (*client) {
		chat::ChatClient cli {};

		std::string name;
		std::cout << "Enter your name: " << std::endl;
		std::cin >> name;

		cli.set_name(name);
		cli.run();
		cli.event_loop();

		return 0;
	}

	return 0;
}
