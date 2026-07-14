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
	test_net_socket.cpp
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
