set(sources
	test_Compan_logger_sink.cpp
	test_Compan_logger_dispatcher.cpp
	test_Compan_logger.cpp
	test_Compan_logger_binder.cpp
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

add_executable(Compan_logger_gtest ${sources} ${headers})

target_compile_options(Compan_logger_gtest PRIVATE -Wall -Wextra -Werror)

target_include_directories(Compan_logger_gtest PRIVATE
	$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
	$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}/Compan_logger>
)


target_link_libraries(Compan_logger_gtest Compan_logger ${libraries})

add_test(Compan_logger_gtest Compan_logger_gtest)

if(NOT CMAKE_CROSSCOMPILING)
add_custom_command(TARGET Compan_logger_gtest POST_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/Compan_logger_gtest -d)
endif()
install(TARGETS Compan_logger_gtest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
