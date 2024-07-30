set(headers
	company_ref_asio_msg_handler.h
	company_ref_asio_msg_handler_factory.h
	company_ref_asio_connection.h
	company_ref_asio_server.h
	company_ref_asio_uds_connection.h
	company_ref_asio_uds_server.h
	company_ref_asio_tcp_connection.h
	company_ref_asio_tcp_server.h
	)
	
set(sources
	company_ref_asio_msg_handler.cpp
	company_ref_asio_msg_handler_factory.cpp
	company_ref_asio_connection.cpp
	company_ref_asio_server.cpp
	company_ref_asio_uds_connection.cpp
	company_ref_asio_uds_server.cpp
	company_ref_asio_tcp_connection.cpp
	company_ref_asio_tcp_server.cpp
	)

add_library(company_ref_asio ${company_ref_asio_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_asio
PUBLIC
	Compan_logger
	company_ref_utils
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_asio
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
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asioConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_asioConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asioConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_asio
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asioConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asioConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_asio
)

install(TARGETS company_ref_asio
	EXPORT company_ref_asio-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_asio-export
	FILE company_ref_asioTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_asio
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_asio)
