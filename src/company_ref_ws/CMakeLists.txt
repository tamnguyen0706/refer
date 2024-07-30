set(sources
	company_ref_ws_main_app.cpp
	company_ref_ws_main.cpp
	)

add_executable(company_ref_ws ${sources})
target_link_libraries(company_ref_ws
	Boost::boost
	Threads::Threads
	company_ref_asio
	company_ref_asio_protocol_server
	Compan_logger
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_dmo
	company_ref_utils
	company_ref_main_apps
	company_ref_dynamic_dmo
	company_ref_sdk_value_ids
	stdc++
	Boost::filesystem
	)
if(${STATIC_ENABLED})
target_link_libraries(company_ref_ws
    atomic
    )
endif()
target_include_directories(company_ref_ws
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	)

install(TARGETS company_ref_ws
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
