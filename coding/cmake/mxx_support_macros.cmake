##########################################################
# This file is used for 'update' and 'notify' 2015/06/17
##########################################################

set(MXX_CMAKE_DIR ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "mxx cmake script directory")

############### Variable definition ###################

if("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Windows")
	set(OS_WIN TRUE)
	set(DSK_WIN TRUE)
elseif("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Linux")
	set(OS_LINUX TRUE)
	set(OS_UNIX TRUE)
	set(DSK_X11 TRUE)
elseif("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Darwin")
	set(OS_MAC TRUE)
	set(OS_UNIX TRUE)
	set(DSK_COCOA TRUE)
elseif("!${CMAKE_SYSTEM_NAME}" STREQUAL "!FreeBSD")
	set(OS_BSD TRUE)
	set(OS_UNIX TRUE)
	set(DSK_X11 TRUE)
else()
	message(FATAL_ERROR "Unknown platform: ${CMAKE_SYSTEM_NAME}!")
endif()

############# Macro definition ##############

macro(mxx_link_wininet)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support wininet.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Wininet.lib")
endmacro()

macro(mxx_link_Ws2_32)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support Ws2_32.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Ws2_32.lib")
endmacro()

macro(mxx_link_wldap32)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support wldap32.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Wldap32.lib")
endmacro()

macro(mxx_link_Urlmon)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support Urlmon.lib.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Urlmon.lib")
endmacro()

macro(mxx_link_wbemuuid)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support wbemuuid.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "wbemuuid.lib")
endmacro()

# mxx_set_runtime_library_type(type)    #设置运行时库类型如MD MT，Debug版会自动追加d
macro(mxx_set_runtime_library_type _type)
	set(CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE "${_type}")
	if("!${CMAKE_BUILD_TYPE}" STREQUAL "!Debug")
		set(CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE "${_type}d")
	endif()
endmacro()

macro(mxx_set_executable_entrypoint ep)
	set(MXX_EXECUTABLE_ENTRYPOINT ${ep})
endmacro()

macro(mxx_set_pgo)
	set(MXX_CURRENT_USE_PGO "yes")
endmacro()

macro(mxx_add_subdirectory)
	set(_argn ${ARGN})

	cmake_parse_arguments(WAS "NONE;WIN;LINUX;MAC;UNIX;BSD" "" "" ${_argn})
	if(NOT WAS_WIN AND NOT WAS_LINUX AND NOT WAS_MAC AND NOT WAS_UNIX AND NOT WAS_NONE)
		set(WAS_WIN ON)
		set(WAS_LINUX ON)
		set(WAS_MAC ON)
		set(WAS_BSD ON)
	endif()
	if(WAS_UNIX)
		set(WAS_LINUX ON)
		set(WAS_MAC ON)
		set(WAS_BSD ON)
	endif()

	set(_valid NO)
	if(OS_WIN)
		set(_valid ${WAS_WIN})
	elseif(OS_LINUX)
		set(_valid ${WAS_LINUX})
	elseif(OS_MAC)
		set(_valid ${WAS_MAC})
	elseif(OS_BSD)
		set(_valid ${WAS_BSD})
	endif()
	
	if(_valid)
		set(_dir ${WAS_UNPARSED_ARGUMENTS})
		if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}" OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}.alt")
			if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}"
				AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}/CMakeLists.txt")
				add_subdirectory(${_dir})
			endif()
			if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}.alt" 
				AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}.alt/CMakeLists.txt")
				add_subdirectory("${_dir}.alt")
				if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_dir}")
					message("use ${_dir}.alt instead of ${_dir}")
				endif()
			endif()
		else()
			message(FATAL_ERROR "${_dir} does not exists.")
		endif()
	endif()
endmacro()

macro(mxx_add_install)
	set(_argn ${ARGN})
	math(EXPR MXX_CURRENT_PACKAGE_INTALL_ID "${MXX_CURRENT_PACKAGE_INTALL_ID}+1")
	set(MXX_ADD_INSTALL_PACKAGES "${MXX_ADD_INSTALL_PACKAGES};${MXX_CURRENT_PACKAGE_NAME}${MXX_CURRENT_PACKAGE_INTALL_ID}"
		CACHE INTERNAL "" FORCE)
	set(MXX_PKG_INST_CFG_${MXX_CURRENT_PACKAGE_NAME}${MXX_CURRENT_PACKAGE_INTALL_ID} 
		${_argn} FILEPATH ${MXX_CURRENT_PACKAGE_FULLPATH} PKG_SUFFIX ${MXX_CURRENT_PACKAGE_SUFFIX}
		CACHE INTERNAL "" FORCE)
endmacro()

macro(mxx_find_split_symbol)
	if(NOT MXX_SPLIT_SYMBOL)
		set(MXX_SPLIT_SYMBOL "${MXX_CMAKE_DIR}/scripts/split_symbol.sh")
	endif()
endmacro()

# mxx_extern_package(_pkg_name _pkg_type
#					<LOCATION _location>
#					[IMP_LOCATION _imp_location]
#					[BINARY_NAMES _name]
#					[BINARY_NAMES_DEBUG _name_dbg]
#					[BINARY_NAMES_RELEASE _name_rls]
#					[PUBLIC_HEADER _pub_hdr]
#					[DEPENDS ...]
#					[MXX_ADD_INSTALL] #将当前package加入到指定安装包中
#其它安装包参数参见宏mxx_add_install 如EXCLUDE CATEGORY LANGUAGE PRODUCT SETOUTPATH 
#					[OUTPUT_DESTDIR]
macro(mxx_extern_package _pkgname _type)
	# a bug on imported target? imported target could not referenced by other file.
	set(_args ${ARGN})
	cmake_parse_arguments(
		MXX_EXTERN_PACKAGE 
		"MXX_ADD_INSTALL;EXCLUDE" 
		"OUTPUT_DESTDIR;LOCATION;IMP_LOCATION;SETOUTPATH" 
		"PUBLIC_HEADER;DEPENDS;CATEGORY;LANGUAGE;PRODUCT;BINARY_NAMES;BINARY_NAMES_DEBUG;BINARY_NAMES_RELEASE" 
		${_args}
		)
	
	set(_names ${MXX_EXTERN_PACKAGE_BINARY_NAMES} ${MXX_EXTERN_PACKAGE_BINARY_NAMES_${BUILD_TYPE_UPPER}})
	if(NOT MXX_EXTERN_PACKAGE_IMP_LOCATION)
		set(MXX_EXTERN_PACKAGE_IMP_LOCATION ${MXX_EXTERN_PACKAGE_LOCATION})
	endif()
	
	if(MXX_EXTERN_PACKAGE_OUTPUT_DESTDIR)
		set(_package_destdir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${MXX_EXTERN_PACKAGE_OUTPUT_DESTDIR}")
	else()
		set(_package_destdir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	endif()
	
	set(_output_files)
	foreach(_name ${_names})
		if(OS_WIN)
			set(_location "${MXX_EXTERN_PACKAGE_LOCATION}/${_name}.dll")
			set(_implib "${MXX_EXTERN_PACKAGE_IMP_LOCATION}/${_name}.lib")
		elseif(OS_LINUX OR OS_BSD)
			set(_so_file_name "lib${_name}.so")
			set(_location "${MXX_EXTERN_PACKAGE_LOCATION}/lib${_name}.so")
			set(_implib ${_location})
		else()
			message(FATAL_ERROR "todo...")
		endif()

		if(MSVC)
			get_filename_component(_filename_we "${_location}" NAME_WE)
			get_filename_component(_location_path "${_location}" PATH)
			if(EXISTS "${_location_path}/${_filename_we}.pdb")
				set(_setup_dbg_symbol_command COMMAND ${CMAKE_COMMAND} -E copy ${_location_path}/${_filename_we}.pdb ${_package_destdir}/${_filename_we}.pdb)
			endif()
		endif()

		if(OS_LINUX)
			mxx_find_split_symbol()
			set(_setup_dbg_symbol_command COMMAND ${MXX_SPLIT_SYMBOL} ${_so_file_name})
		endif()
		
		if("${_type}!" STREQUAL "SHARED!" OR "${_type}!" STREQUAL "MODULE!")
			get_filename_component(_filename "${_location}" NAME)
			if(OS_LINUX OR OS_BSD)
				if(NOT EXISTS ${_package_destdir})
					file(MAKE_DIRECTORY "${_package_destdir}")
				endif()
				set(_cp_command COMMAND cp -R ${_location}* ${_package_destdir})
			else()
				set(_cp_command COMMAND ${CMAKE_COMMAND} -E copy ${_location} ${_package_destdir})
			endif()
			if(NOT EXISTS "${_location}")
				add_custom_command(
					OUTPUT ${_package_destdir}/${_filename}
					${_cp_command}
					${_setup_dbg_symbol_command}
					WORKING_DIRECTORY ${_package_destdir}
					#MAIN_DEPENDENCY "${_location}"
					)
			else()
				add_custom_command(
					OUTPUT ${_package_destdir}/${_filename}
					${_cp_command}
					${_setup_dbg_symbol_command}
					WORKING_DIRECTORY ${_package_destdir}
					MAIN_DEPENDENCY "${_location}"
					)
			endif()
			
			list(APPEND _output_files ${_package_destdir}/${_filename})
		endif()
	endforeach()
	
	if("${_type}!" STREQUAL "STATIC!")
		add_custom_target(${_pkgname})
	elseif("${_type}!" STREQUAL "SHARED!" OR "${_type}!" STREQUAL "MODULE!")
		add_custom_target(
			${_pkgname} ALL
			DEPENDS ${_output_files}
			)
	else()
		message(FATAL_ERROR "${_type} is not a valid extern package type.")
	endif()

	if(MXX_EXTERN_PACKAGE_DEPENDS)
		add_dependencies(${_pkgname} ${MXX_EXTERN_PACKAGE_DEPENDS})
	endif()

	if(MXX_EXTERN_PACKAGE_PUBLIC_HEADER)
		set_property(TARGET ${_pkgname} 
			PROPERTY PUBLIC_HEADER_DIR "${MXX_EXTERN_PACKAGE_PUBLIC_HEADER}")
	endif()

	set_property(TARGET ${_pkgname} PROPERTY IMPORTED_LOCATION "${_location}")
	set_property(TARGET ${_pkgname} PROPERTY IMPORTED_IMPLIB "${_implib}")

	set_property(TARGET ${_pkgname} PROPERTY MXX_PACKAGE_TYPE "EXTERN")
	
	if(MXX_EXTERN_PACKAGE_MXX_ADD_INSTALL)
		foreach(var ${_output_files})
			math(EXPR MXX_CURRENT_PACKAGE_INTALL_ID "${MXX_CURRENT_PACKAGE_INTALL_ID}+1")
			set(MXX_ADD_INSTALL_PACKAGES "${MXX_ADD_INSTALL_PACKAGES};${_pkgname}${MXX_CURRENT_PACKAGE_INTALL_ID}"
				CACHE INTERNAL "" FORCE)
			set(_exclude)
			if(MXX_EXTERN_PACKAGE_EXCLUDE)
				set(_exclude "EXCLUDE")
			endif()
			set(MXX_PKG_INST_CFG_${_pkgname}${MXX_CURRENT_PACKAGE_INTALL_ID} 
					${_exclude} 
					CATEGORY ${MXX_EXTERN_PACKAGE_CATEGORY} 
					LANGUAGE ${MXX_EXTERN_PACKAGE_LANGUAGE} 
					PRODUCT ${MXX_EXTERN_PACKAGE_PRODUCT} 
					SETOUTPATH ${MXX_EXTERN_PACKAGE_SETOUTPATH} 
					RENAME ${MXX_EXTERN_PACKAGE_RENAME} 
					FILEPATH ${var}
					PKG_SUFFIX ${_pkg_suffix} 
					CACHE INTERNAL "" FORCE)
		endforeach()
	endif()
endmacro()

macro(mxx_find_mxxenv)
	if(NOT MXX_MXXENV_PATH)
		if(OS_WIN)
			get_filename_component(
				MXX_MXXENV_PATH 
				"[HKEY_CURRENT_USER\\Software\\kingsoft\\Office\\mxxenv;qt-kso-integration]" 
				ABSOLUTE 
				)
			if(NOT MXX_MXXENV_PATH OR NOT EXISTS "${MXX_MXXENV_PATH}/register.bat")
				message(FATAL_ERROR "Can not found mxxenv! "
					"Use register.bat to register mxxenv to system or special MXX_MXXENV_PATH manually.")
			endif()
			message("mxxenv path: ${MXX_MXXENV_PATH}")
			set(MXX_MXXENV_PATH "${MXX_MXXENV_PATH}" CACHE PATH "Path to mxxenv library")
		else()
			file(STRINGS "$ENV{HOME}/.config/Kingsoft/mxxenv/qt-kso-integration" MXX_MXXENV_PATH)
			if(NOT MXX_MXXENV_PATH OR NOT EXISTS "${MXX_MXXENV_PATH}/register.sh")
				message(FATAL_ERROR "Can not found mxxenv! "
					"Use register.sh to register mxxenv to system or special MXX_MXXENV_PATH manually.")
			endif()
			message("mxxenv path: ${MXX_MXXENV_PATH}")
			set(MXX_MXXENV_PATH "${MXX_MXXENV_PATH}" CACHE PATH "Path to mxxenv library")
		endif()
		mark_as_advanced(MXX_MIDL)
	endif()
endmacro()

macro(mxx_link_ATL)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support ATL.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "winspool.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "comdlg32.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "advapi32.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "shell32.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "ole32.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "oleaut32.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "uuid.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "odbc32.lib")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "odbccp32.lib")
endmacro()

macro(mxx_link_shlwapi)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support shlwapi.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "shlwapi.lib")
endmacro()

macro(mxx_link_NetApi32)
	if(NOT WIN32)
		message(FATAL_ERROR "Only windows support NetApi32.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Netapi32.lib")
endmacro()

macro(mxx_link_version)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support version.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "version.lib")
endmacro()

macro(mxx_link_Psapi)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support Psapi.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Psapi.lib")
endmacro()

macro(mxx_link_crypt32)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support crypt32.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Crypt32.lib")
endmacro()

macro(mxx_link_wintrust)
	if(NOT OS_WIN)
		message(FATAL_ERROR "Only windows support wintrust.")
	endif()
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "WinTrust.lib")
endmacro()

set(MXX_CURRENT_PACKAGE_INTALL_ID 0)
set(MXX_ADD_INSTALL_PACKAGES "" CACHE INTERNAL "mxx install patch file list")