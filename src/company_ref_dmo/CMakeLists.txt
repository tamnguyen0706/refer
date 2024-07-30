set(headers
	company_ref_dmo_container.h
	company_ref_dmo_file.h
	company_ref_dmo_helper.h
	)
set(sources
	company_ref_dmo_container.cpp
	company_ref_dmo_file.cpp
	company_ref_dmo_helper.cpp
	)

add_library(company_ref_dmo ${company_ref_dmo_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_dmo
PUBLIC
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_utils
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_dmo
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
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_dmoConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_dmoConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_dmoConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_dmo
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_dmoConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_dmoConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_dmo
)

install(TARGETS company_ref_dmo
	EXPORT company_ref_dmo-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_dmo-export
	FILE company_ref_dmoTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_dmo
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_dmo)
