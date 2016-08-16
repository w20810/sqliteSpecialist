cmake_minimum_required(VERSION 2.8.0)

include(cmake/precheck.cmake)
project(sqliteSpecialist)
include(cmake/mxx.cmake)
include(cmake/mxx_support_macros.cmake)


	message("Win32 compile!")
#	add_subdirectory(cmake)
	add_subdirectory(curl)
	add_subdirectory(openssl)
	add_subdirectory(zlib)
	add_subdirectory(shell)
	add_subdirectory(uilib)
#	add_subdirectory(wke)
	add_subdirectory(resources)
	add_subdirectory(sqlite)
final_deal()