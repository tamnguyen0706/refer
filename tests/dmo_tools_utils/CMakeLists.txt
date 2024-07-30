include(CMakePrintHelpers)

set(sources
	test_dmo_tools_utils_mock.cpp
	test_Compan_dmo_ini_converter.cpp
	test_dmo_from_trxml_dmdocument.cpp
	test_Compan_dmo_valueid_generator.cpp
	test_Compan_dmo_valueid_generator_class_def.cpp
	test_Compan_dmo_valueid_generator_class_impl.cpp
	test_Compan_dmo_meta_generator_class_def.cpp
	test_Compan_dmo_meta_generator_class_impl.cpp
)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xml_snippets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_executable(dmo_tools_utils_gtest ${sources})

target_compile_options(dmo_tools_utils_gtest
	PRIVATE
		-Wall
		-Werror
		-Wextra
		-Wno-deprecated-declarations
)

target_include_directories(dmo_tools_utils_gtest
	PUBLIC
		$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(dmo_tools_utils_gtest
	PUBLIC
		Compan_gtest
		Boost::boost
		stdc++
		Threads::Threads
		dmo_tools_utils
)

add_test(dmo_tools_utils_gtest dmo_tools_utils_gtest)

if(NOT CMAKE_CROSSCOMPILING)
add_custom_command(TARGET dmo_tools_utils_gtest POST_BUILD
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/dmo_tools_utils_gtest -d)
endif()

install(TARGETS dmo_tools_utils_gtest
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
