add_library(tcpchat
	chat_server.cpp

	net_socket.cpp
	net_unix.cpp
)

target_include_directories(tcpchat PUBLIC
	${CMAKE_SOURCE_DIR}/include
)
