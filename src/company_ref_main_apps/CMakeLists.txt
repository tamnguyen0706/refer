set(headers
	company_ref_app_options.h
	company_ref_main_io_context.h
	Compan_logger_factory.h
	)
set(sources
	company_ref_app_options.cpp
	company_ref_main_io_context.cpp
	Compan_logger_factory.cpp
	Compan_logger_binder_variantvalue.cpp
	)

add_library(company_ref_main_apps ${company_ref_main_apps_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_main_apps
PUBLIC
	Boost::boost
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_main_apps
	PUBLIC
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		${PROJECT_INCLUDE_DIR}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_main_appsConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_main_appsConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_main_appsConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_main_apps
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_main_appsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_main_appsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_main_apps
)

install(TARGETS company_ref_main_apps
	EXPORT company_ref_main_apps-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_main_apps-export
	FILE company_ref_main_appsTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_main_apps
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_main_apps)
