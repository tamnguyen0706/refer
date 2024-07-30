set(sources
	company_ref_cli_bl.cpp
	company_ref_cli_command.cpp
	company_ref_cli_container.cpp
	company_ref_cli_invoke.cpp
	company_ref_cli_main.cpp
	company_ref_cli_output.cpp
	company_ref_value_io_helper.cpp
	)

add_executable(company_ref_cli ${sources})
target_link_libraries(company_ref_cli
	Boost::boost
	Boost::filesystem
	Threads::Threads
	company_ref_boost_client
	Compan_logger
	company_ref_protocol_utils
	company_ref_utils
	stdc++
	)
if(${STATIC_ENABLED})
target_link_libraries(company_ref_cli
    atomic
    )
endif()
target_include_directories(company_ref_cli
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS company_ref_cli
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
