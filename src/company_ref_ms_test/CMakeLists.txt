set(sources
	test_base.cpp
	test_subscribe.cpp
	test_container.cpp
	ms_test_microservice.cpp
	company_ref_ms_test_main.cpp
	)

add_executable(company_ref_ms_test ${sources})
target_link_libraries(company_ref_ms_test
	Boost::boost
	Threads::Threads
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_dmo
	company_ref_boost_client
	company_ref_microservice
	company_ref_utils
	company_ref_main_apps
	stdc++
	Boost::filesystem
	)
target_include_directories(company_ref_ms_test
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS company_ref_ms_test
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
