add_executable(tests_tcpchat
	test_chat_server.cpp
)

target_include_directories(tests_tcpchat
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tests_tcpchat
	tcpchat

	GTest::GTest
	GTest::Main
	pthread
)

##

add_executable(tests_net_utils
	test_net_utils.cpp
	test_net_socket.cpp
	test_net_unix.cpp
	test_net_io.cpp
)

target_include_directories(tests_net_utils
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(tests_net_utils
	tcpchat

	GTest::GTest
	GTest::Main
	pthread
)

##

add_executable(test_server
	test_server_client_connection/test_server.cpp
)

target_include_directories(test_server
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_server
	tcpchat
)

##

add_executable(test_client
	test_server_client_connection/test_client.cpp
)

target_include_directories(test_client
	PRIVATE
	${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(test_client
	tcpchat
)
