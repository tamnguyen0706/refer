set(headers
	company_ref_microservice_message_handler.h
	company_ref_microservice_connection.h
	company_ref_microservice_client.h
	company_ref_microservice_dynamic_dmo.h
	)
set(sources
	company_ref_microservice_message_handler.cpp
	company_ref_microservice_connection.cpp
	company_ref_microservice_client.cpp
	company_ref_microservice_dynamic_dmo.cpp
	)

add_library(company_ref_microservice ${company_ref_microservice_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_microservice
PUBLIC
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_asio
	company_ref_asio_protocol_client
	company_ref_utils
	company_ref_main_apps
	company_ref_dynamic_dmo
	company_ref_sdk_value_ids
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_microservice
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
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_microserviceConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_microserviceConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_microserviceConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_microservice
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_microserviceConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_microserviceConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_microservice
)

install(TARGETS company_ref_microservice
	EXPORT company_ref_microservice-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_microservice-export
	FILE company_ref_microserviceTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_microservice
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_microservice)
