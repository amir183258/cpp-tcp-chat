add_library(tcpchat
	chat_server.cpp
	chat_client.cpp

	net_socket.cpp
	net_unix.cpp
	net_io.cpp

	protocol_buffer.cpp
)

target_include_directories(tcpchat PUBLIC
	${CMAKE_SOURCE_DIR}/include
)
