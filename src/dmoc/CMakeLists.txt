set(sources
	dmoc_main.cpp
	)

add_executable(dmoc ${sources})
target_link_libraries(dmoc
	Boost::boost
	Boost::filesystem
	Threads::Threads
	Compan_logger
	company_ref_main_apps
	company_ref_utils
	dmo_tools_utils
	stdc++
	)


target_include_directories(dmoc
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS dmoc
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
