set(headers
	company_ref_variant_bool_value.h
	company_ref_variant_dinterval_value.h
	company_ref_variant_enum_value.h
	company_ref_variant_eui48_value.h
	company_ref_variant_factory.h
	company_ref_variant_interval_base.h
	company_ref_variant_interval_value.h
	company_ref_variant_ipv4_value.h
	company_ref_variant_ipv6_value.h
	company_ref_variant_llinterval_value.h
	company_ref_variant_map_value.h
	company_ref_variant_sinterval_value.h
	company_ref_variant_text_value.h
	company_ref_variant_timespec_value.h
	company_ref_variant_timeval_value.h
	company_ref_variant_uinterval_value.h
	company_ref_variant_ullinterval_value.h
	company_ref_variant_usinterval_value.h
	company_ref_variant_udid_value.h
	company_ref_variant_container_util.h
	company_ref_variant_container_base.h
	company_ref_variant_vector_value.h
	company_ref_variant_set_value.h
	company_ref_variant_unorderedset_value.h
	company_ref_variant_valuestore_dispatcher.h
	company_ref_variant_valuestore.h
	company_ref_variant_valuestore_hash_bucket.h
	company_ref_variant_valuestore_hash_methods.h
	company_ref_variant_valuestore_hash_token.h
	company_ref_variant_valuestore_hashtoken_map.h
	company_ref_variant_valuestore_hashtoken_set.h
	company_ref_variant_valuestore_valuedata.h
	company_ref_variant_valuestore_value_id_bucketizer.h
	company_ref_variant_valuestore_valueid.h
	company_ref_variant_valuestore_variant.h
	company_ref_variant_valuestore_visitor.h
	)
set(sources
	company_ref_variant_bool_value.cpp
	company_ref_variant_dinterval_value.cpp
	company_ref_variant_enum_value.cpp
	company_ref_variant_eui48_value.cpp
	company_ref_variant_factory.cpp
	company_ref_variant_interval_value.cpp
	company_ref_variant_ipv4_value.cpp
	company_ref_variant_ipv6_value.cpp
	company_ref_variant_llinterval_value.cpp
	company_ref_variant_map_value.cpp
	company_ref_variant_sinterval_value.cpp
	company_ref_variant_text_value.cpp
	company_ref_variant_timespec_value.cpp
	company_ref_variant_timeval_value.cpp
	company_ref_variant_uinterval_value.cpp
	company_ref_variant_ullinterval_value.cpp
	company_ref_variant_usinterval_value.cpp
	company_ref_variant_udid_value.cpp
	company_ref_variant_container_util.cpp
	company_ref_variant_container_base.cpp
	company_ref_variant_vector_value.cpp
	company_ref_variant_set_value.cpp
	company_ref_variant_unorderedset_value.cpp
	company_ref_variant_valuestore.cpp
	company_ref_variant_valuestore_dispatcher.cpp
	company_ref_variant_valuestore_hash_bucket.cpp
	company_ref_variant_valuestore_hash_methods.cpp
	company_ref_variant_valuestore_hash_token.cpp
	company_ref_variant_valuestore_hashtoken_set.cpp
	company_ref_variant_valuestore_valuedata.cpp
	company_ref_variant_valuestore_value_id_bucketizer.cpp
	company_ref_variant_valuestore_valueid.cpp
	company_ref_variant_valuestore_variant.cpp
	company_ref_variant_valuestore_visitor.cpp
	)

add_library(company_ref_variant_valuestore ${company_ref_variant_valuestore_LIBRARY_TYPE} ${sources})
target_link_libraries(company_ref_variant_valuestore
PUBLIC
	Compan_logger
	company_ref_protocol
	company_ref_protocol_utils
	company_ref_utils
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(company_ref_variant_valuestore
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
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_variant_valuestoreConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_variant_valuestoreConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_variant_valuestoreConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_variant_valuestore
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_variant_valuestoreConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_variant_valuestoreConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_variant_valuestore
)

install(TARGETS company_ref_variant_valuestore
	EXPORT company_ref_variant_valuestore-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_variant_valuestore-export
	FILE company_ref_variant_valuestoreTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_variant_valuestore
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_variant_valuestore)
