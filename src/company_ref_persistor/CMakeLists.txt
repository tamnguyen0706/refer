set(sources
	company_ref_persistor_bl.cpp
	company_ref_persistor_client.cpp
	company_ref_persistor_main.cpp
	)

add_executable(company_ref_persistor ${sources})
target_link_libraries(company_ref_persistor
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
	)
target_include_directories(company_ref_persistor
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS company_ref_persistor
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
