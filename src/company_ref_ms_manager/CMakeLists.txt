set(sources
	company_ref_ms_manager_main.cpp
	company_ref_ms_controller.cpp
	company_ref_ms_applet.cpp
	)

add_executable(company_ref_ms_manager ${sources})
target_link_libraries(company_ref_ms_manager
	Boost::boost
	Threads::Threads
	company_ref_boost_client
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_microservice
	company_ref_utils
	company_ref_sdk_value_ids
	company_ref_main_apps
	stdc++
	)
target_include_directories(company_ref_ms_manager
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS company_ref_ms_manager
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
