set(sources
	dmo_show.cpp
	)

add_executable(dmo_show ${sources})
target_link_libraries(dmo_show
	Boost::boost
	Threads::Threads
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_dmo
	company_ref_utils
	company_ref_main_apps
	stdc++
	)

target_include_directories(dmo_show
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS dmo_show
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
