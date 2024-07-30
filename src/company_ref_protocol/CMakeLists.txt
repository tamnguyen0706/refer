protobuf_generate_cpp(Compan_PROTO_SRCS Compan_PROTO_HDRS Compan.proto)
protobuf_generate_cpp(edge_PROTO_SRCS edge_PROTO_HDRS company_ref_protocol.proto)
protobuf_generate_cpp(dmo_PROTO_SRCS dmo_PROTO_HDRS company_ref_datamodel.proto)


if (${absl_FOUND})
set(libraries ${libraries}
	absl::absl_check
	absl::absl_log
	absl::algorithm
	absl::base
	absl::bind_front
	absl::bits
	absl::btree
	absl::cleanup
	absl::cord
	absl::core_headers
	absl::debugging
	absl::die_if_null
	absl::dynamic_annotations
	absl::flags
	absl::flat_hash_map
	absl::flat_hash_set
	absl::function_ref
	absl::hash
	absl::layout
	absl::log_initialize
	absl::log_severity
	absl::memory
	absl::node_hash_map
	absl::node_hash_set
	absl::optional
	absl::span
	absl::status
	absl::statusor
	absl::strings
	absl::synchronization
	absl::time
	absl::type_traits
	absl::utility
	absl::variant
	${utf8_range}
	${utf8_validity}
)
endif()

add_library(company_ref_protocol ${company_ref_protocol_LIBRARY_TYPE}
	${Compan_PROTO_SRCS} ${Compan_PROTO_HDRS}
	${edge_PROTO_SRCS} ${edge_PROTO_HDRS}
	${dmo_PROTO_SRCS} ${dmo_PROTO_HDRS}
	)

target_compile_options(company_ref_protocol PUBLIC -Wno-error=unused-parameter)

target_link_libraries(company_ref_protocol PUBLIC ${Protobuf_LITE_LIBRARIES}
	${libraries}
	PRIVATE
	${libatomic}
)

# >=gcc-12 bug: causes false positive: MEM[(struct Storage *)&indents].data_.allocated.allocated_data may be uninitialized
if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 12.0)
target_link_libraries(company_ref_protocol PUBLIC -Wno-error=maybe-uninitialized)
endif()

target_include_directories(company_ref_protocol
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${Protobuf_INCLUDE_DIR}
)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/company_ref_protocolConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_protocolConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_protocol
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_protocolConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/company_ref_protocolConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_protocol
)

install(TARGETS company_ref_protocol
	EXPORT company_ref_protocol-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT company_ref_protocol-export
	FILE company_ref_protocolTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/company_ref_protocol
)

write_basic_package_version_file(
       ${CMAKE_CURRENT_BINARY_DIR}/company_ref_protocolConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)
install(FILES ${Compan_PROTO_HDRS} ${edge_PROTO_HDRS} ${dmo_PROTO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/company_ref_protocol)
