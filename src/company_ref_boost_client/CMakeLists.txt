set(headers
	company_ref_boost_client.h
	company_ref_boost_tcp_client.h
	company_ref_boost_uds_client.h
	)
set(sources
	company_ref_boost_client.cpp
	company_ref_boost_tcp_client.cpp
	company_ref_boost_uds_client.cpp
	)

add_library(company_ref_boost_client ${company_ref_boost_client_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_boost_client
PUBLIC
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_boost_client
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
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_clientConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_boost_clientConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_clientConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_boost_client
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_clientConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_boost_clientConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_boost_client
)

install(TARGETS company_ref_boost_client
	EXPORT company_ref_boost_client-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_boost_client-export
	FILE company_ref_boost_clientTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_boost_client
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_boost_client)
