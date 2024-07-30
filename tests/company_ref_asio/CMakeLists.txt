set(sources
	test_company_ref_asio_msg_handler.cpp
	test_company_ref_asio_connection.cpp
	test_company_ref_asio_uds_server.cpp
)

function(add_sources sources_var headers_var libraries_var)
	if(UNIT_TESTING)
		list(APPEND ${sources_var} ${mock_sources})
		list(APPEND ${sources_var} ${test_sources})
		list(APPEND ${headers_var} ${mock_headers})
		list(APPEND ${headers_var} ${test_headers})
		list(APPEND ${libraries_var} Compan_gtest)
	endif()
	if(IT_TESTING)
		list(APPEND ${sources_var} ${it_sources})
		list(APPEND ${headers_var} ${it_headers})
	endif()
	if(OS_LINUX)
		list(APPEND ${sources_var} ${linux_sources})
		list(APPEND ${headers_var} ${linux_headers})
	endif()
	if(OS_DARWIN)
		list(APPEND ${sources_var} ${darwin_sources})
		list(APPEND ${headers_var} ${darwin_headers})
	endif()
	list(SORT ${headers_var})
	list(SORT ${sources_var})
	set(${sources_var} "${${sources_var}}" PARENT_SCOPE)
	set(${headers_var} "${${headers_var}}" PARENT_SCOPE)
	set(${libraries_var} "${${libraries_var}}" PARENT_SCOPE)
endfunction(add_sources)

add_sources(sources headers libraries)

add_executable(company_ref_asio_gtest ${sources} ${headers})

target_compile_options(company_ref_asio_gtest PRIVATE -Wall -Wextra -Werror)

target_include_directories(company_ref_asio_gtest PRIVATE
	$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}/Compan_logger>
	$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}/company_ref_asio>
)

target_link_libraries(company_ref_asio_gtest company_ref_asio Compan_logger ${libraries})

add_test(company_ref_asio_gtest company_ref_asio_gtest)

if(NOT CMAKE_CROSSCOMPILING)
add_custom_command(TARGET company_ref_asio_gtest POST_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/company_ref_asio_gtest -d)
endif()
install(TARGETS company_ref_asio_gtest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
