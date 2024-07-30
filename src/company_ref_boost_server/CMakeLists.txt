set(headers
	company_ref_boost_message_handler.h
	company_ref_boost_handler_factory.h
	company_ref_boost_ws_handler_factory.h
	company_ref_boost_ws_message_handler.h
	company_ref_boost_server.h
	company_ref_boost_server_connection.h
	company_ref_boost_server_connection_manager.h
	company_ref_boost_tcp_server.h
	company_ref_boost_uds_server.h
	)
set(sources
	company_ref_boost_message_handler.cpp
	company_ref_boost_handler_factory.cpp
	company_ref_boost_ws_handler_factory.cpp
	company_ref_boost_ws_message_handler.cpp
	company_ref_boost_server.cpp
	company_ref_boost_server_connection.cpp
	company_ref_boost_server_connection_manager.cpp
	company_ref_boost_tcp_server.cpp
	company_ref_boost_uds_server.cpp
	)

add_library(company_ref_boost_server ${company_ref_boost_server_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_boost_server
PUBLIC
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_utils
	company_ref_dmo
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_boost_server
	PUBLIC
		$<BUILD_INTERFACE:${Protobuf_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		${PROJECT_INCLUDE_DIR}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_serverConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_boost_serverConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_serverConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_boost_server
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_serverConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_serverConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_boost_server
)

install(TARGETS company_ref_boost_server
	EXPORT company_ref_boost_server-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_boost_server-export
	FILE company_ref_boost_serverTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_boost_server
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_boost_server)
