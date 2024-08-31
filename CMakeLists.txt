cmake_minimum_required(VERSION 3.20)
project(jtag-tools)


add_library(mpsse mpsse.c)
target_include_directories(mpsse PUBLIC .)
target_link_libraries(mpsse ftdi)

add_library(jtag jtag_tap.c)
target_include_directories(jtag PUBLIC .)
target_link_libraries(jtag mpsse)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/GlobalLogger_sqlite.c)
  add_library(GlobalLogger_sqlite SHARED GlobalLogger_sqlite.c )
else()
  message(FATAL_ERROR "You must copy the generated files GlobalLogger.h and GlobalLogger_sqlite.c to this directory to build this tool.")
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/GlobalLogger_printer.c)
  add_library(GlobalLogger_printer SHARED GlobalLogger_printer.c )
else()
  message(FATAL_ERROR "You must copy the generated files GlobalLogger.h and GlobalLogger_sqlite.c to this directory to build this tool.")
endif()

add_executable(jtag_log_capture jtag_log_capture.c)
target_link_libraries(jtag_log_capture mpsse jtag)

add_executable(sqlite_logger sqlite_logger.c)
target_link_libraries(sqlite_logger sqlite3 GlobalLogger_sqlite)

add_executable(print_logger print_logger.c)
target_link_libraries(print_logger GlobalLogger_printer)
