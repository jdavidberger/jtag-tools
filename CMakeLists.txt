cmake_minimum_required(VERSION 3.20)
project(jtag-tools)


add_library(mpsse mpsse.c)
target_include_directories(mpsse PUBLIC .)
target_link_libraries(mpsse ftdi)

add_library(jtag jtag_tap.c)
target_include_directories(jtag PUBLIC .)
target_link_libraries(jtag mpsse)

if(EXISTS GlobalLogger_sqlite.c)
  add_library(GlobalLogger_sqlite SHARED GlobalLogger_sqlite.c )
else()
  message(FATAL_ERROR "You must copy the generated files GlobalLogger.h and GlobalLogger_sqlite.c to this directory to build this tool.")
endif()

add_executable(sqlite_logger sqlite_logger.c)
target_link_libraries(sqlite_logger mpsse jtag sqlite3 GlobalLogger_sqlite)
