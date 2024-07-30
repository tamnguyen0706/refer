set(headers
	Compan_dmo_ini_converter.h
	Compan_dmo_from_trxml_dmdocument.h
	Compan_dmo_valueid_generator.h
	Compan_dmo_valueid_generator_class_def.h
	Compan_dmo_valueid_generator_class_impl.h
	Compan_dmo_meta_generator_class_def.h
	Compan_dmo_meta_generator_class_impl.h
	)
set(sources
	Compan_dmo_ini_converter.cpp
	Compan_dmo_from_trxml_dmdocument.cpp
	Compan_dmo_valueid_generator.cpp
	Compan_dmo_valueid_generator_class_def.cpp
	Compan_dmo_valueid_generator_class_impl.cpp
	Compan_dmo_meta_generator_class_def.cpp
	Compan_dmo_meta_generator_class_impl.cpp
	)

add_library(dmo_tools_utils ${dmo_tools_utils_LIBRARY_TYPE} ${sources})
target_link_libraries(dmo_tools_utils
PUBLIC
	Boost::boost
	Boost::filesystem
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_variant_valuestore
	company_ref_dmo
	company_ref_utils
	Compan_logger
	stdc++
	Threads::Threads
)

target_include_directories(dmo_tools_utils
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
       ${CMAKE_CURRENT_BINARY_DIR}/dmo_tools_utilsConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/dmo_tools_utilsConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/dmo_tools_utilsConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dmo_tools_utils
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/dmo_tools_utilsConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/dmo_tools_utilsConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dmo_tools_utils
)

install(TARGETS dmo_tools_utils
	EXPORT dmo_tools_utils-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT dmo_tools_utils-export
	FILE dmo_tools_utilsTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/dmo_tools_utils
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dmo_tools_utils)
