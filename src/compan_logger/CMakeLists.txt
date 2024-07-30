set(headers
	Compan_logger_loglevel.h
	Compan_logger_stream.h
	Compan_logger.h
	Compan_logger_dispatcher.h
	Compan_logger_dispatcher_asio.h
	Compan_logger_format_helper.h
	Compan_logger_sink.h
	Compan_logger_sink_cout.h
	Compan_logger_sink_buffered.h
	Compan_logger_sink_syslog.h
	Compan_logger_controller.h
	Compan_logger_binder.h
	)
set(sources
	Compan_logger_loglevel.cpp
	Compan_logger_stream.cpp
	Compan_logger.cpp
	Compan_logger_dispatcher.cpp
	Compan_logger_dispatcher_asio.cpp
	Compan_logger_sink.cpp
	Compan_logger_format_helper.cpp
	Compan_logger_sink_cout.cpp
	Compan_logger_sink_buffered.cpp
	Compan_logger_sink_syslog.cpp
	Compan_logger_controller.cpp
	Compan_logger_binder.cpp
	)

add_library(Compan_logger ${Compan_logger_LIBRARY_TYPE} ${sources})
target_link_libraries(Compan_logger
PUBLIC
	Boost::boost
	stdc++
	Threads::Threads
)

target_include_directories(Compan_logger
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
       ${CMAKE_CURRENT_BINARY_DIR}/Compan_loggerConfigVersion.cmake
       VERSION ${PROJECT_VERSION}
       COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Compan_loggerConfig.cmake.in
       ${CMAKE_CURRENT_BINARY_DIR}/Compan_loggerConfig.cmake
       INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Compan_logger
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/Compan_loggerConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Compan_loggerConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Compan_logger
)

install(TARGETS Compan_logger
	EXPORT Compan_logger-export
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT Compan_logger-export
	FILE Compan_loggerTargets.cmake
	NAMESPACE Compan::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Compan_logger
)

install(FILES
	${headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Compan_logger)
