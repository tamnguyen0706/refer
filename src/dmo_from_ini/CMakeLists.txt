set(sources
	dmo_from_ini_main.cpp
	)

add_executable(dmo_from_ini ${sources})
target_link_libraries(dmo_from_ini
	Boost::boost
	Threads::Threads
	Compan_logger
	company_ref_main_apps
	company_ref_utils
	dmo_tools_utils
	stdc++
	)

target_include_directories(dmo_from_ini
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS dmo_from_ini
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
