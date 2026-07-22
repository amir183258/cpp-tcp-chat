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

##
add_executable(chat
	tcpchat.cpp
)

target_include_directories(chat
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(chat
	tcpchat
)
