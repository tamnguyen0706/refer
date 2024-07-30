set(headers
	company_ref_asio_server_protocol_handler.h
	company_ref_asio_server_protocol_serializer.h
	company_ref_asio_server_msg_handler_factory.h
	)
set(sources
	company_ref_asio_server_protocol_handler.cpp
	company_ref_asio_server_protocol_serializer.cpp
	company_ref_asio_server_msg_handler_factory.cpp
	)

add_library(company_ref_asio_protocol_server ${company_ref_asio_protocol_server_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_asio_protocol_server
PUBLIC
	Compan_logger
	company_ref_asio
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_utils
	company_ref_dmo
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_asio_protocol_server
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
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asio_protocol_serverConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_asio_protocol_serverConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asio_protocol_serverConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_asio_protocol_server
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asio_protocol_serverConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asio_protocol_serverConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_asio_protocol_server
)

install(TARGETS company_ref_asio_protocol_server
	EXPORT company_ref_asio_protocol_server-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_asio_protocol_server-export
	FILE company_ref_asio_protocol_serverTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_asio_protocol_server
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_asio_protocol_server)
