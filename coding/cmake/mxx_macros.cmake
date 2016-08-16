include(CMakeParseArguments)

# find _radio_options in _input and then remove it in _input
# if no found _default is returned 
# if more then one found error occused
# example: _token_radio_option(ARGN "AAA|BBB|CC" "BBB" _result)
function(_token_radio_option _input _radio_options _default _output)
	set(_l ${${_input}})
	string(REGEX MATCHALL "[^|]+" _opts "${_radio_options}")
	set(_nhit 0)
	set(_hit)
	foreach(_x ${_l})
		list(FIND _opts "${_x}" _idx)
		if(NOT _idx EQUAL -1)
			math(EXPR _nhit "${_nhit}+1")
			set(_hit "${_x}")
		endif()
	endforeach()
	if(_nhit EQUAL 0)
		set(${_output} ${_default} PARENT_SCOPE)
	elseif(_nhit EQUAL 1)
		set(${_output} ${_hit} PARENT_SCOPE)
		list(REMOVE_ITEM _l "${_hit}")
		set(${_input} ${_l} PARENT_SCOPE)
	else()
		message(FATAL_ERROR "radio option check error!")
	endif()
endfunction()

macro(_impl_args_system_filter _filtered_args_ref)
	set(_argn ${ARGN})
	set(_a)
	while(NOT "!${_argn}" STREQUAL "!")#不用while(_argn)是为了避免_argn为0, OFF, NO, FALSE, N, IGNORE, or ends in the suffix '-NOTFOUND'情况，导致表达式结果为假（非期望）
		_token_list_element(_argn 0 _a)
		if(_a MATCHES "(^WIN$|^UNIX$|^LINUX$|^MAC$|^BSD$)")
			list(GET _argn 0 _b)
			if("!${_b}" STREQUAL "!(")
				list(REMOVE_AT _argn 0)
				if(OS_${_a})
					_impl_args_system_filter(${_filtered_args_ref} ${_argn})
				else()
					_del_args()
				endif()
			else()
				list(APPEND ${_filtered_args_ref} "${_a}")
			endif()
		elseif("!${_a}" STREQUAL "!)")
			break()
		else()
			list(APPEND ${_filtered_args_ref} "${_a}")
		endif()
	endwhile()
endmacro()

macro(_args_system_filter _filtered_args_ref)
	set(${_filtered_args_ref})
	_impl_args_system_filter(${_filtered_args_ref} ${ARGN})
endmacro()

macro(_token_list_element _tle_list_ref _tle_list_index _tle_element_ref)
	list(GET ${_tle_list_ref} ${_tle_list_index} ${_tle_element_ref})
	list(REMOVE_AT ${_tle_list_ref} ${_tle_list_index})
endmacro()

macro(_del_args)#递归删除()内的参数 用递归来处理括号嵌套 即保证“(a(b)c)”中c)也能被删掉
	_token_list_element(_argn 0 _a)
	while(NOT "!${_a}" STREQUAL "!)")
		if("!${_a}" STREQUAL "!(")
			_del_args()
		endif()
		_token_list_element(_argn 0 _a)
	endwhile()
endmacro()

macro(_get_cur_pkg_main_output)
	if("${CURRENT_PACKAGE_TYPE}!" STREQUAL "SHARED!")
		set(_filename "${CMAKE_SHARED_LIBRARY_PREFIX}${CURRENT_PACKAGE_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(CURRENT_PACKAGE_FULLPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_filename}")
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "MODULE!")
		set(_filename "${CMAKE_SHARED_MODULE_PREFIX}${CURRENT_PACKAGE_NAME}${CMAKE_SHARED_MODULE_SUFFIX}")
		set(CURRENT_PACKAGE_FULLPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_filename}")
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "EXECUTABLE!" 
			OR "${CURRENT_PACKAGE_TYPE}!" STREQUAL "CONSOLE!")
		set(CURRENT_PACKAGE_FULLPATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CURRENT_PACKAGE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "LNG_LEGACY!")	
		_get_project_name_and_locale(_prj_name _locale)
		set(CURRENT_PACKAGE_FULLPATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mui/${_locale}/${_prj_name}.lng")
	endif()
	
	get_filename_component(CURRENT_PACKAGE_SUFFIX "${CURRENT_PACKAGE_FULLPATH}" EXT)
	get_filename_component(CURRENT_PACKAGE_FILENAME "${CURRENT_PACKAGE_FULLPATH}" NAME)
endmacro()

# mxx_package(<name>
				#[NONE|STATIC|SHARED|MODULE|EXECUTABLE|CONSOLE|LNG_LEGACY]    #package类型
				#[BUILD_DEST_PATH <destpath>]    #package的二进制文件输出路径，默认值为office6目录下，如传入BUILD_DEST_PATH dir1则会在顶层输出路径下生成dir1并在此文件夹内生成二进制文件 即dir1将和office6同级
				#)
macro(mxx_package _name)
	cmake_parse_arguments(_MXX 
		"" 
		"BUILD_DEST_PATH" 
		"" 
		${ARGN})
	if(DEFINED _MXX_BUILD_DEST_PATH)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${MXX_OUTPUT_ROOT_DIRECOTRY}/${_MXX_BUILD_DEST_PATH}")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MXX_OUTPUT_ROOT_DIRECOTRY}/${_MXX_BUILD_DEST_PATH}")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	endif()
	
	set(_argn ${_MXX_UNPARSED_ARGUMENTS})
	set(pkg_types "NONE|STATIC|SHARED|MODULE|EXECUTABLE|CONSOLE|LNG_LEGACY")
	_token_radio_option(_argn ${pkg_types} "NONE" _type)
	
	if(_argn)
		message(FATAL_ERROR "\nthe package type your set could not find in TypeList.\nThe correct TypeList is ${pkg_types}\n")
	endif()
	set(CURRENT_PACKAGE_NAME "${_name}")
	set(CURRENT_PACKAGE_TYPE "${_type}")
	set(CURRENT_PACKAGE_SRCS)
	set(CURRENT_PACKAGE_INCLUDE_PKG_DIRS)
	set(CURRENT_PACKAGE_LINK_PKGS)
	set(CURRENT_PACKAGE_LINK_LIBRARYS)
	set(CURRENT_PACKAGE_DEPEND_PKGS)
	set(CURRENT_PACKAGE_HAS_PUBLIC_HEADER)
	set(CURRENT_PACKAGE_GENERATEE_FILES)
	set(CURRENT_PACKAGE_DEFINITIONS)
	set(CURRENT_PACKAGE_RESES)
	
	set(CURRENT_PACKAGE_FULLPATH)
	set(CURRENT_PACKAGE_FILENAME)
	set(CURRENT_PACKAGE_SUFFIX)
	
	set(EXECUTABLE_ENTRYPOINT "wWinMain")
	set(CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE "MD")
	if("!${CMAKE_BUILD_TYPE}" STREQUAL "!Debug")
		set(CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE "MDd")
	endif()
	set(CURRENT_PACKAGE_OPTIMIZATION "${CXX_OPTIMIZATION_MINSIZE_FLAGS}")
	if("!${CMAKE_BUILD_TYPE}" STREQUAL "!Debug")
		set(CURRENT_PACKAGE_OPTIMIZATION "${CXX_OPTIMIZATION_NONE_FLAGS}")
	endif()
	
	mxx_include_directories(${CMAKE_CURRENT_BINARY_DIR})
	mxx_include_directories("${CMAKE_BINARY_DIR}/public_header")
	_get_cur_pkg_main_output()
endmacro(mxx_package)

# mxx_package_ex 当指定BUILD_DEST_PATH，可将输出路径指定为BUILD_DEST_PATH的值，不再受到MXX_OUTPUT_ROOT_DIRECOTRY限制
macro(mxx_package_ex _name)
	cmake_parse_arguments(_MXX 
		"" 
		"BUILD_DEST_PATH" 
		"" 
		${ARGN})
	if(DEFINED _MXX_BUILD_DEST_PATH)
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_MXX_BUILD_DEST_PATH}")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
		set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_MXX_BUILD_DEST_PATH}")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
		set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
	endif()
	
	set(_argn ${_MXX_UNPARSED_ARGUMENTS})
	set(pkg_types "NONE|STATIC|SHARED|MODULE|EXECUTABLE|CONSOLE|LNG_LEGACY")
	_token_radio_option(_argn ${pkg_types} "NONE" _type)
	
	if(_argn)
		message(FATAL_ERROR "\nthe package type your set could not find in TypeList.\nThe correct TypeList is ${pkg_types}\n")
	endif()
	set(CURRENT_PACKAGE_NAME "${_name}")
	set(CURRENT_PACKAGE_TYPE "${_type}")
	set(CURRENT_PACKAGE_SRCS)
	set(CURRENT_PACKAGE_INCLUDE_PKG_DIRS)
	set(CURRENT_PACKAGE_LINK_PKGS)
	set(CURRENT_PACKAGE_LINK_LIBRARYS)
	set(CURRENT_PACKAGE_DEPEND_PKGS)
	set(CURRENT_PACKAGE_HAS_PUBLIC_HEADER)
	set(CURRENT_PACKAGE_GENERATEE_FILES)
	set(CURRENT_PACKAGE_DEFINITIONS)
	set(CURRENT_PACKAGE_RESES)
	
	set(CURRENT_PACKAGE_FULLPATH)
	set(CURRENT_PACKAGE_FILENAME)
	set(CURRENT_PACKAGE_SUFFIX)
	
	set(EXECUTABLE_ENTRYPOINT "wWinMain")
	set(CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE "MD")
	if("!${CMAKE_BUILD_TYPE}" STREQUAL "!Debug")
		set(CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE "MDd")
	endif()
	set(CURRENT_PACKAGE_OPTIMIZATION "${CXX_OPTIMIZATION_MINSIZE_FLAGS}")
	if("!${CMAKE_BUILD_TYPE}" STREQUAL "!Debug")
		set(CURRENT_PACKAGE_OPTIMIZATION "${CXX_OPTIMIZATION_NONE_FLAGS}")
	endif()
	
	mxx_include_directories(${CMAKE_CURRENT_BINARY_DIR})
	mxx_include_directories("${CMAKE_BINARY_DIR}/public_header")
	_get_cur_pkg_main_output()
endmacro(mxx_package_ex)

function(_get_pdb_output_fullpath _output)
	#.. currently, cmake could not change pdb's output directory!!! crazy...
	string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type)
	set(_output_dir_type)
	if(CURRENT_PACKAGE_TYPE STREQUAL "STATIC")
		set(_output_dir_type "ARCHIVE")
	elseif(CURRENT_PACKAGE_TYPE STREQUAL "SHARED" OR CURRENT_PACKAGE_TYPE STREQUAL "MODULE")
		set(_output_dir_type "LIBRARY")
	elseif(CURRENT_PACKAGE_TYPE STREQUAL "EXECUTABLE" OR CURRENT_PACKAGE_TYPE STREQUAL "CONSOLE")
		set(_output_dir_type "RUNTIME")
	endif()
	set(${_output} "${CMAKE_${_output_dir_type}_OUTPUT_DIRECTORY}/${CURRENT_PACKAGE_NAME}.pdb" PARENT_SCOPE)
endfunction()

macro(_mxx_deal_pch _srcs_ref _pch_header _pch_source _idl_gen_ref)
	set(_mxx_deal_pch_srcs ${${_srcs_ref}})
	if(MSVC)
		if(CMAKE_GENERATOR MATCHES "^Visual Studio")
			foreach(_x ${_mxx_deal_pch_srcs})
				if(_x MATCHES "(\\.cpp$|\\.cc$|\\.cxx$)")
					set_property(SOURCE "${_x}" PROPERTY COMPILE_FLAGS "/Yu\"${_pch_header}\"")
				endif()
			endforeach()
			list(APPEND _mxx_deal_pch_srcs ${_pch_header} ${_pch_source})
			set_property(SOURCE "${_pch_source}" PROPERTY COMPILE_FLAGS "/Yc\"${_pch_header}\"")
		else()
			set(_pch_pch ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CURRENT_PACKAGE_NAME}.dir/${_pch_header}.pch)
			set(_pch_obj ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CURRENT_PACKAGE_NAME}.dir/${_pch_source}.obj)
			_get_compile_args(CXX _args)
			_get_pdb_output_fullpath(_pdb_pos)
			foreach(_x ${_mxx_deal_pch_srcs})
				if(_x MATCHES "(\\.cpp$|\\.cc$|\\.cxx$)")
					get_property(_flags SOURCE "${_x}" PROPERTY COMPILE_FLAGS)
					set_property(SOURCE "${_x}" 
						PROPERTY COMPILE_FLAGS "${_flags} /Yu\"${_pch_header}\" /Fp\"${_pch_pch}\"")
					get_property(_obj_dep SOURCE "${_x}" PROPERTY OBJECT_DEPENDS)
					if(_obj_dep)
						set(_obj_dep "${_obj_dep};")
					endif()
					set_property(SOURCE "${_x}" PROPERTY OBJECT_DEPENDS "${_obj_dep}${_pch_pch}")
				endif()
			endforeach()
			list(APPEND _mxx_deal_pch_srcs ${_pch_header}) # pch's source do not add back to src list
			list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "${_pch_obj}")
			add_custom_command(
				OUTPUT ${_pch_pch} ${_pch_obj}
				COMMAND ${CMAKE_CXX_COMPILER} /c ${_args} ${CURRENT_PACKAGE_OPTIMIZATION} /Yc\"${_pch_header}\" 
						/Fp\"${_pch_pch}\" /Fd\"${_pdb_pos}\" /Fo\"${_pch_obj}\"
						"${CMAKE_CURRENT_SOURCE_DIR}/${_pch_source}"
				MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${_pch_source}
				IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${_pch_source}
				DEPENDS ${${_idl_gen_ref}} # some of pch header depends idl generated file, so...
				COMMENT "Creating Precompiler header..."
				)
		endif()
	endif()
	set(${_srcs_ref} ${_mxx_deal_pch_srcs})
endmacro()

macro(_mxx_deal_sources_group _srcs_ref _idl_gen_ref)
	set(_wdsg_srcs ${${_srcs_ref}})
	set(_pch_header "")
	set(_pch_source "")

	list(FIND _wdsg_srcs "PCH" _pch_idx)
	if(NOT _pch_idx EQUAL -1)
		list(REMOVE_AT _wdsg_srcs ${_pch_idx})
		list(GET _wdsg_srcs ${_pch_idx} _pch_header)
		list(REMOVE_AT _wdsg_srcs ${_pch_idx})
		list(GET _wdsg_srcs ${_pch_idx} _pch_source)
		list(REMOVE_AT _wdsg_srcs ${_pch_idx})
	endif()

	if(_pch_header)
		_mxx_deal_pch(_wdsg_srcs ${_pch_header} ${_pch_source} ${_idl_gen_ref})
	endif()	

	set(${_srcs_ref} ${_wdsg_srcs})
endmacro()

macro(_mxx_deal_sources _srcs_ref)
	set(_wds_srcs ${${_srcs_ref}})
	set(_srcs_for_compile)
	set(_srcs_grouped)
	foreach(_src ${_wds_srcs})
		if(NOT _src STREQUAL "SOURCE_SEPARATOR")
			list(APPEND _srcs_grouped ${_src})
			if(IS_ABSOLUTE ${_src})
				file(RELATIVE_PATH _group_path ${CMAKE_CURRENT_SOURCE_DIR} ${_src})
			else()
				file(RELATIVE_PATH _group_path ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${_src})
			endif()
			get_filename_component(_group_path ${_group_path} PATH)
			file(TO_NATIVE_PATH "${_group_path}" _group_path)
			source_group("${_group_path}" FILES ${_src})
		else()
			if(_srcs_grouped)
				_mxx_deal_sources_group(_srcs_grouped _wds_idl_gen)
				list(APPEND _srcs_for_compile ${_srcs_grouped})
			endif()
			set(_srcs_grouped)
		endif()
	endforeach()

	list(APPEND _srcs_for_compile ${_wds_export_symbols_gen} ${_wds_idl_gen} ${_wds_uic_gen} ${_wds_qrc_gen} ${_wds_ts_gen})
	set(${_srcs_ref} ${_srcs_for_compile})
endmacro()

macro(_gen_res_stamp _res_ref _res_stamp)
	set(_res_list ${${_res_ref}})
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
		COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
		DEPENDS ${_res_list}
		)
	source_group(res\\stamp FILES ${CMAKE_CURRENT_BINARY_DIR}/res.stamp)
	set(${_res_stamp} ${CMAKE_CURRENT_BINARY_DIR}/res.stamp)
endmacro()

macro(_append_target_property_string _target _property _val)
	get_target_property(_orig_val ${_target} ${_property})
	if(_orig_val)
		set_target_properties(${_target} PROPERTIES ${_property} "${_orig_val} ${_val}")
	else()
		set_target_properties(${_target} PROPERTIES ${_property} "${_val}")
	endif()
endmacro()

# mxx_end_package()
macro(mxx_end_package)
	_mxx_generate_binary_info_file()
	set(_srcs_for_compile ${CURRENT_PACKAGE_SRCS})
	_mxx_deal_sources(_srcs_for_compile)

	if(CURRENT_PACKAGE_RESES)
		_gen_res_stamp(CURRENT_PACKAGE_RESES _res_gen)
		list(APPEND _srcs_for_compile ${_res_gen})
	endif()

	if("${CURRENT_PACKAGE_TYPE}!" STREQUAL "STATIC!")
		add_library(${CURRENT_PACKAGE_NAME} STATIC ${_srcs_for_compile})
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "SHARED!")
		add_library(${CURRENT_PACKAGE_NAME} SHARED ${_srcs_for_compile})
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "MODULE!")
		add_library(${CURRENT_PACKAGE_NAME} MODULE ${_srcs_for_compile})
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "EXECUTABLE!")
		add_executable(${CURRENT_PACKAGE_NAME} WIN32 ${_srcs_for_compile})
		if(EXECUTABLE_ENTRYPOINT)
			_append_target_property_string(${CURRENT_PACKAGE_NAME} LINK_FLAGS " /ENTRY:\"${EXECUTABLE_ENTRYPOINT}CRTStartup\" ")
		endif()
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "CONSOLE!")
		add_executable(${CURRENT_PACKAGE_NAME} ${_srcs_for_compile})
	elseif("${CURRENT_PACKAGE_TYPE}!" STREQUAL "NONE!")
		add_custom_target(${CURRENT_PACKAGE_NAME} ALL
		   DEPENDS ${_srcs_for_compile} ${CURRENT_PACKAGE_GENERATEE_FILES}
		   SOURCES ${_srcs_for_compile}
		   )
	endif()
	
	_append_target_property_string(${CURRENT_PACKAGE_NAME} 
			COMPILE_FLAGS " ${CURRENT_PACKAGE_OPTIMIZATION} ")
	_append_target_property_string(${CURRENT_PACKAGE_NAME} 
			COMPILE_FLAGS " /${CURRENT_PACKAGE_RUNTIME_LIBRARY_TYPE} ")

	if(DEFINED CURRENT_PACKAGE_DELAYLOAD_FILENAME)
		_append_target_property_string(${CURRENT_PACKAGE_NAME} 
				LINK_FLAGS " /DELAYLOAD:${CURRENT_PACKAGE_DELAYLOAD_FILENAME} ")
	endif()
	
	if("${CURRENT_PACKAGE_TYPE}!" STREQUAL "EXECUTABLE!" OR
		"${CURRENT_PACKAGE_TYPE}!" STREQUAL "CONSOLE!")
		if(UAC_EXECUTION_LEVEL)
			_append_target_property_string(${CURRENT_PACKAGE_NAME} 
					LINK_FLAGS " /level='${UAC_EXECUTION_LEVEL}' ")#cmake有bug 必须这样写http://www.cmake.org/Bug/print_bug_page.php?bug_id=11171
		endif()
		if(CURRENT_PACKAGE_DEP)
			_append_target_property_string(${CURRENT_PACKAGE_NAME} LINK_FLAGS " /${CURRENT_PACKAGE_DEP} ")
		endif()
	endif()
	
	if (DEFINED CURRENT_EXTRA_LINK_FLAGS)
		_append_target_property_string(${CURRENT_PACKAGE_NAME} 
				LINK_FLAGS " ${CURRENT_EXTRA_LINK_FLAGS}")
	endif()
	
	if (DEFINED CMAKE_USE_LTCG AND
		DEFINED CURRENT_USE_PGO AND
		"!${CMAKE_BUILD_TYPE}" STREQUAL "!Release"
		)
		_append_target_property_string(${CURRENT_PACKAGE_NAME} 
					COMPILE_FLAGS " /GL /Gy")

		if("${CURRENT_PACKAGE_TYPE}!" STREQUAL "EXECUTABLE!" OR
			"${CURRENT_PACKAGE_TYPE}!" STREQUAL "SHARED!")
			if("!${CMAKE_USE_LTCG}" STREQUAL "!PGInstrument")
				_append_target_property_string(${CURRENT_PACKAGE_NAME} 
						LINK_FLAGS " /LTCG:PGInstrument")
			elseif("!${CMAKE_USE_LTCG}" STREQUAL "!PGOptimize")
				_append_target_property_string(${CURRENT_PACKAGE_NAME} 
						LINK_FLAGS " /LTCG:PGOptimize")
			else()
				_append_target_property_string(${CURRENT_PACKAGE_NAME} 
						LINK_FLAGS " /LTCG")
			endif()
		endif()
	endif()
	
	if(CURRENT_PACKAGE_DEFINITIONS)
		set_property(TARGET ${CURRENT_PACKAGE_NAME} 
				PROPERTY COMPILE_DEFINITIONS "${CURRENT_PACKAGE_DEFINITIONS}")
	endif()

	if(CURRENT_PACKAGE_LINK_LIBRARYS)
		target_link_libraries(${CURRENT_PACKAGE_NAME} ${CURRENT_PACKAGE_LINK_LIBRARYS})
	endif()
	
	if(CURRENT_PACKAGE_LINK_PKGS)
		set(PACKAGE_LINK_DEPENDS ${PACKAGE_LINK_DEPENDS} 
			${CURRENT_PACKAGE_NAME}
			${CURRENT_PACKAGE_LINK_PKGS}
			"PACKAGE_LINK_DEPENDS_SEPARATOR"
			CACHE INTERNAL "" FORCE)
	endif()
	
	if(CURRENT_PACKAGE_DEPEND_PKGS)
		add_dependencies(${CURRENT_PACKAGE_NAME} ${CURRENT_PACKAGE_DEPEND_PKGS})
	endif()

	if(CURRENT_PACKAGE_HAS_PUBLIC_HEADER)
		set(_pub_dir "${CMAKE_CURRENT_BINARY_DIR}/include")
		set_property(TARGET ${CURRENT_PACKAGE_NAME} PROPERTY PUBLIC_HEADER_DIR "${_pub_dir}")
	endif()

	set_property(TARGET ${CURRENT_PACKAGE_NAME} PROPERTY MXX_PACKAGE_TYPE "${CURRENT_PACKAGE_TYPE}")

	set(MXX_PACKAGE_INFO_LIST "${MXX_PACKAGE_INFO_LIST}${CURRENT_PACKAGE_NAME}    ${CURRENT_PACKAGE_TYPE}    ${CMAKE_CURRENT_SOURCE_DIR};" CACHE INTERNAL "" FORCE)
endmacro()

macro(mxx_add_phony_target _targetname)
	add_custom_target(${_targetname})
	add_dependencies(${_targetname} ${ARGN})
endmacro()

macro(mxx_add_compile_flags _opt)
	set(WPS_CURRENT_EXTRA_COMPILE_FLAGS ${_opt})
endmacro()

# mxx_add_sources([PCH header.h source.cpp] a.h a.cpp b.h b.cpp ...)
macro(mxx_add_sources)
	_args_system_filter(_cur_system_argn ${ARGN})
	foreach(_f ${_cur_system_argn})
		if(_f MATCHES "(PCH)")
		else()
			string(TOLOWER "${_f}" _f)
			if(_f MATCHES "(\\.c|\\.cc|\\.cpp|\\.cxx)")
			elseif(_f MATCHES "(\\.h|\\.hh|\\.hpp|\\.hxx)")
			elseif(_f MATCHES "(\\.inl)")
			elseif(_f MATCHES "(\\.rc|\\.idl|\\.rc|\\.def)")
			elseif(_f MATCHES "(\\.ui|\\.qrc|\\.ts)")
			else()
				message(FATAL_ERROR "Unrecognized source file ${_f}")
			endif()
		endif()
	endforeach()
	list(APPEND CURRENT_PACKAGE_SRCS ${_cur_system_argn} "SOURCE_SEPARATOR")
endmacro()

# mxx_include_directories(dir1 dir2 ...)
macro(mxx_include_directories)
	_args_system_filter(_cur_system_argn ${ARGN})
	include_directories(${_cur_system_argn})
endmacro()

# mxx_add_definitions(def1 def2 ...)
macro(mxx_add_definitions)
	_args_system_filter(_cur_system_argn ${ARGN})
	list(APPEND CURRENT_PACKAGE_DEFINITIONS ${_cur_system_argn})
endmacro()

macro(mxx_include_packages)
	_args_system_filter(_cur_system_argn ${ARGN})
	foreach(_pkg ${_cur_system_argn})
		if(NOT TARGET ${_pkg})
			message(FATAL_ERROR "${_pkg} is not a valid package.")
		endif()
		get_property(_pub_dir TARGET ${_pkg} PROPERTY PUBLIC_HEADER_DIR)
		if(NOT _pub_dir)
			message(WARNING "${_pkg} do not has public header.")
		endif()
		include_directories(${_pub_dir})
		list(APPEND CURRENT_PACKAGE_DEPEND_PKGS ${_pkg})
		list(APPEND CURRENT_PACKAGE_INCLUDE_PKG_DIRS ${_pub_dir})
	endforeach()
endmacro()

macro(mxx_link_packages)
	_args_system_filter(_cur_system_argn ${ARGN})
	list(APPEND CURRENT_PACKAGE_LINK_PKGS ${_cur_system_argn})
endmacro()

macro(_deal_resource_file _res_ref _res_path_list_ref _res_verify_cmd _res_setup_cmd)
	if(NOT "!${_res_setup_cmd}" STREQUAL "!" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		list(FIND SETUP_CMD_LIST "${_res_setup_cmd}" _res_setup_index)
		if(_res_setup_index EQUAL -1)
			message(FATAL_ERROR "${_res_setup_cmd} is not a supported command.\nsupport commands: ${SETUP_CMD_LIST}.")	
		endif()
		set(_res_command ${_res_setup_cmd})
	else()
		set(_res_command ${CMAKE_COMMAND} -E copy)
	endif()
	if(NOT "!${_res_verify_cmd}" STREQUAL "!")
		list(FIND VERIFY_CMD_LIST "${_res_verify_cmd}" _res_verify_index)
		if(_res_verify_index EQUAL -1)
			message(FATAL_ERROR "${_res_verify_cmd} is not a supported command.\nsupport commands: ${VERIFY_CMD_LIST}.")
		endif()
	endif()
	set(_res_list ${${_res_ref}})
	set(_res_gen)
	foreach(item ${_res_list})
		set(_res_verify_command)
		if(NOT "!${_res_verify_cmd}" STREQUAL "!")
			if(DAILY_BUILD_MODE)
				set(_res_verify_command ${_res_verify_cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${item} --output-warning)
			else()
				set(_res_verify_command ${_res_verify_cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${item} --output-error)
			endif()
		endif()
		set(_src_file ${CMAKE_CURRENT_SOURCE_DIR}/${item})
		set(_dest_file ${CURRENT_RESES_GROUP_DESTPATH}/${item})
        
    set(_cur_command ${_res_command})
    if(IS_DIRECTORY ${_src_file})
			set(_cur_command ${CMAKE_COMMAND} -E copy_directory)
    endif()
        
		add_custom_command(
			OUTPUT ${CURRENT_RESES_GROUP_DESTPATH}/${item}
			COMMAND ${_res_verify_command}
			COMMAND ${_cur_command} ${_src_file} ${_dest_file}
			MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${item}
			)
		list(APPEND _res_gen ${CURRENT_RESES_GROUP_DESTPATH}/${item})
	endforeach()
	source_group(res FILES ${_res_list})
	set(${_res_path_list_ref} ${_res_gen})
endmacro()

# mxx_add_resources(a.png b.png ...
				#[DELAY_GENERATE <c.png ...>]    #c.png等文件在cmake配置阶段还不存在，但需要打入安装包则使用DELAY_GENERATE，对于delphi等非cmake编译生成的文件需要指定此参数
				#[BUILD_DEST_PATH <destpath>]    #生成资源文件的路径，默认值为office6目录下，如传入BUILD_DEST_PATH dir1则会在顶层输出路径下生成dir1文件夹并拷贝资源
				#[WPS_ADD_INSTALL]    #将当前package加入到指定安装包中，此参数及相关参数需放在该函数末尾
				#[VERIFY_COMMAND <verify_cmd>]    #如果需要检查资源文件，指定verify_cmd为相应的检查命令,如检查xml缩进格式的xmlchecker
				#[SETUP_COMMAND <setup_cmd>]    #指定用复制以外的命令处理资源文件，指定setup_cmd为相应的命令,如对xml文件复制时，可用命令xmlformatter进行不带注释部分地复制xml文件
				#[TEMPLATE]    #如果该组需打包的资源文件为模板文件则需指定此参数，模板文件通常在templates目录下
				#[IMGLIB]    #如果该组需打包的资源文件为剪贴画则需指定此参数，剪贴画通常在imglib目录下
				#其它安装包参数参见宏wps_add_install 如EXCLUDE CATEGORY LANGUAGE PRODUCT SETOUTPATH 此处不支持: RENAME 
				#)
macro(mxx_add_resources)
	_args_system_filter(_cur_system_argn ${ARGN})
	cmake_parse_arguments(_WAR 
		"WPS_ADD_INSTALL;EXCLUDE;TEMPLATE;IMGLIB" 
		"RENAME;SETOUTPATH;BUILD_DEST_PATH;VERIFY_COMMAND;SETUP_COMMAND" 
		"CATEGORY;LANGUAGE;PRODUCT;DELAY_GENERATE"
		${_cur_system_argn})
	#CMAKE中macro里的变量为全局变量，使用前先置空，防止上次设的值对这次有影响
	set(_exclude)
	set(_template)
	set(_imglib)
	if(_WAR_EXCLUDE)
		set(_exclude "EXCLUDE")
	endif()
	if(_WAR_TEMPLATE)
		set(_template "TEMPLATE")
	endif()
	if(_WAR_IMGLIB)
		set(_imglib "IMGLIB")
	endif()
	if(DEFINED _WAR_BUILD_DEST_PATH)
		set(CURRENT_RESES_GROUP_DESTPATH "${MXX_OUTPUT_ROOT_DIRECOTRY}/${_WAR_BUILD_DEST_PATH}")
	else()
		set(CURRENT_RESES_GROUP_DESTPATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resource")
	endif()
	foreach(var ${_WAR_UNPARSED_ARGUMENTS})
		if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${var})
			message(FATAL_ERROR "${var} does not exists.")
		endif()
	endforeach()

	_deal_resource_file(_WAR_UNPARSED_ARGUMENTS _res_path_list "${_WAR_VERIFY_COMMAND}" "${_WAR_SETUP_COMMAND}")
	list(APPEND CURRENT_PACKAGE_RESES ${_res_path_list})
endmacro(mxx_add_resources)

macro(_uncache_vars)
	foreach(_v_ref ${ARGN})
		get_property(${_v_ref}_CACHE_TYPE CACHE ${_v_ref} PROPERTY TYPE)
		if(NOT ${_v_ref}_CACHE_TYPE)
			message(FATAL_ERROR "Cache entry: ${_v_ref} not exists.")
		endif()
		get_property(${_v_ref}_CACHE_HELPSTRING CACHE ${_v_ref} PROPERTY HELPSTRING)
		get_property(${_v_ref}_CACHE_ADVANCED CACHE ${_v_ref} PROPERTY ADVANCED)
		set(_tmp ${${_v_ref}})
		unset(${_v_ref} CACHE)
		set(${_v_ref} ${_tmp})
		list(APPEND _UNCACHED_VARIANTS_LIST ${_v_ref})
	endforeach()
endmacro()

macro(_recache_vars)
	foreach(_v_ref ${_UNCACHED_VARIANTS_LIST})
		set(${_v_ref} ${${_v_ref}} CACHE ${${_v_ref}_CACHE_TYPE} ${${_v_ref}_CACHE_HELPSTRING})
		if(${_v_ref}_CACHE_ADVANCED)
			mark_as_advanced(${_v_ref})
		endif()
	endforeach()
endmacro()

macro(_set_compiler_flags)
	if(NOT INITED_COMPILER)
		_uncache_vars(
			CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG CMAKE_EXE_LINKER_FLAGS_RELEASE
			CMAKE_MODULE_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS_DEBUG CMAKE_MODULE_LINKER_FLAGS_RELEASE
			CMAKE_SHARED_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS_DEBUG CMAKE_SHARED_LINKER_FLAGS_RELEASE
			)
		set(INITED_COMPILER YES CACHE BOOL "Flag seted if mxx has init compiler flags.")
		mark_as_advanced(INITED_COMPILER)
	endif()

	if(NOT CMAKE_C_COMPILER_ID STREQUAL CMAKE_CXX_COMPILER_ID)
		message(FATAL_ERROR "c && c++ compiler is not same.")
	endif()
	string(TOLOWER "${CMAKE_C_COMPILER_ID}" _compiler_id)
	if(NOT EXISTS "${MXX_CMAKE_DIR}/${_compiler_id}_flags.cmake")
		message(FATAL_ERROR "${CMAKE_PROJECT_NAME} currently do not support ${CMAKE_CXX_COMPILER_ID}.")
	else()
		include("${MXX_CMAKE_DIR}/${_compiler_id}_flags.cmake")
	endif()

	_recache_vars()
endmacro()

macro(_check_compiler)
	if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release")
		message(FATAL_ERROR "${CMAKE_PROJECT_NAME} currently only support Debug and Release.")
	endif()
	_set_compiler_flags()

	# remove we don't need options
	unset(CMAKE_C_FLAGS_MINSIZEREL CACHE)
	unset(CMAKE_C_FLAGS_RELWITHDEBINFO CACHE)
	unset(CMAKE_CXX_FLAGS_MINSIZEREL CACHE)
	unset(CMAKE_CXX_FLAGS_RELWITHDEBINFO CACHE)
	unset(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL CACHE)
	unset(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO CACHE)
	unset(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL CACHE)
	unset(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO CACHE)
	unset(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL CACHE)
	unset(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO CACHE)
endmacro()

macro(_get_target_implib _gti_pkg _proj _output_ref)
	if(TARGET ${_gti_pkg})
		get_property(_pkg_type TARGET ${_gti_pkg} PROPERTY MXX_PACKAGE_TYPE)
		if(NOT _pkg_type)
			message(FATAL_ERROR "${_gti_pkg} is not a valid source package.Error in ${_proj}")
		endif()
		if("${_pkg_type}!" STREQUAL "STATIC!")
			set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_gti_pkg}.lib")
		elseif("${_pkg_type}!" STREQUAL "SHARED!")
			set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_gti_pkg}.lib")
		elseif("${_pkg_type}!" STREQUAL "EXECUTABLE!")
			set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_gti_pkg}.lib")
		elseif("${_pkg_type}!" STREQUAL "EXTERN!")
			get_property(${_output_ref} TARGET ${_gti_pkg} PROPERTY IMPORTED_IMPLIB)
		elseif("${_pkg_type}!" STREQUAL "NONE!")
			set(${_output_ref})
		else()
			message(FATAL_ERROR "${_gti_pkg} is a ${_pkg_type} package, do not support be linked.")
		endif()
	else()
		message(FATAL_ERROR "Current Project ${_proj} could not find package named ${_gti_pkg}")
	endif()
endmacro()

macro(_deal_link_packages_internal _pkg)
	set(_link_libs)
	get_property(_pkg_compile_flags TARGET ${_pkg} PROPERTY COMPILE_FLAGS)
	string(REGEX MATCH "/M[DT]+" _pkg_runtime_lib_type "${_pkg_compile_flags}")
	foreach(_d_pkg ${ARGN})
		if (NOT TARGET ${_d_pkg})
			message(FATAL_ERROR "package ${_d_pkg} not exist. referenced by ${_pkg}")
		endif()
		get_property(_d_pkg_compile_flags TARGET ${_d_pkg} PROPERTY COMPILE_FLAGS)
		string(REGEX MATCH "/M[DT]+" _d_pkg_runtime_lib_type "${_d_pkg_compile_flags}")
		get_property(_d_pkg_type TARGET ${_d_pkg} PROPERTY MXX_PACKAGE_TYPE)
		if(_d_pkg_runtime_lib_type AND "!${_d_pkg_type}" STREQUAL "!STATIC" AND NOT "!${_pkg_runtime_lib_type}" STREQUAL "!${_d_pkg_runtime_lib_type}")
			#检查package与被link package运行时库类型是否相同 无权限的项目不会被检查
			message(FATAL_ERROR "package ${_pkg} is ${_pkg_runtime_lib_type}, but ${_d_pkg} is ${_d_pkg_runtime_lib_type}.")
		endif()
		_get_target_implib(${_d_pkg} ${_pkg} _implib)
		list(APPEND _link_libs ${_implib})
	endforeach()
	set(_extern_link_arg " ")
	foreach(_link_lib ${_link_libs})
		set(_extern_link_arg "${_extern_link_arg} ${_link_lib} ")
	endforeach()
	_append_target_property_string(${_pkg} LINK_FLAGS ${_extern_link_arg})
	set_property(TARGET ${_pkg} APPEND PROPERTY LINK_DEPENDS ${_link_libs})
	add_dependencies(${_pkg} ${ARGN})
endmacro()

macro(_mxx_deal_link_packages)
	set(_deps)
	foreach(_pkg ${PACKAGE_LINK_DEPENDS})
		if(NOT _pkg STREQUAL "PACKAGE_LINK_DEPENDS_SEPARATOR")
			list(APPEND _deps ${_pkg})
		else()
			_deal_link_packages_internal(${_deps})
			set(_deps)
		endif()
	endforeach()
	unset(PACKAGE_LINK_DEPENDS CACHE)
endmacro()

macro(_get_pkg_filename _pkg_name _pkg_type _filename_ref)
	if("!${_pkg_type}" STREQUAL "!SHARED")
		set(${_filename_ref} "${_pkg_name}${CMAKE_SHARED_LIBRARY_SUFFIX}")
	elseif("!${_pkg_type}" STREQUAL "!MODULE")
		set(${_filename_ref} "${_pkg_name}${CMAKE_SHARED_MODULE_SUFFIX}")
	elseif("!${_pkg_type}" STREQUAL "!LNG_LEGACY")
		set(${_filename_ref} "${_pkg_name}.lng")
	elseif("!${_pkg_type}" STREQUAL "!EXTERN")
		get_property(_extern_pkg_location TARGET ${_delay_pkg} PROPERTY IMPORTED_LOCATION)
		get_filename_component(${_filename_ref} "${_extern_pkg_location}" NAME)
	endif()
endmacro()

macro(_mxx_deal_delayload_packages)
	foreach(_pkgname ${MXX_DELAYLOAD_PACKAGES})
		foreach(_delay_pkg ${PKG_DELAYLOAD_${_pkgname}})
			set(_delay_pkg_filename)
			get_property(_delay_pkg_type TARGET ${_delay_pkg} PROPERTY MXX_PACKAGE_TYPE)
			_get_pkg_filename(${_delay_pkg} ${_delay_pkg_type} _delay_pkg_filename)
			if(_delay_pkg_filename)
				_append_target_property_string(${_pkgname}
					LINK_FLAGS " /DELAYLOAD:${_delay_pkg_filename} ")
			else()
				message(FATAL_ERROR "\"${_pkgname}\" can not delayload package \"${_delay_pkg}\".")
			endif()
		endforeach()
		unset(PKG_DELAYLOAD_${_pkgname} CACHE)
	endforeach()
	unset(MXX_DELAYLOAD_PACKAGES CACHE)
endmacro()

# mxx_extern_package(_pkg_name _pkg_type
#					<LOCATION _location>
#					[IMP_LOCATION _imp_location]
#					[BINARY_NAMES _name]
#					[BINARY_NAMES_DEBUG _name_dbg]
#					[BINARY_NAMES_RELEASE _name_rls]
#					[PUBLIC_HEADER _pub_hdr]
#					[DEPENDS ...]
#					[WPS_ADD_INSTALL] #将当前package加入到指定安装包中
#其它安装包参数参见宏wps_add_install 如EXCLUDE CATEGORY LANGUAGE PRODUCT SETOUTPATH 
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
		set(_location "${MXX_EXTERN_PACKAGE_LOCATION}/${_name}.dll")
		set(_implib "${MXX_EXTERN_PACKAGE_IMP_LOCATION}/${_name}.lib")

		get_filename_component(_filename_we "${_location}" NAME_WE)
		get_filename_component(_location_path "${_location}" PATH)
		if(EXISTS "${_location_path}/${_filename_we}.pdb")
			set(_setup_dbg_symbol_command COMMAND ${CMAKE_COMMAND} -E copy ${_location_path}/${_filename_we}.pdb ${_package_destdir}/${_filename_we}.pdb)
		endif()
		
		if("${_type}!" STREQUAL "SHARED!" OR "${_type}!" STREQUAL "MODULE!")
			get_filename_component(_filename "${_location}" NAME)
			set(_cp_command COMMAND ${CMAKE_COMMAND} -E copy ${_location} ${_package_destdir})
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
			PROPERTY MXX_PUBLIC_HEADER_DIR "${MXX_EXTERN_PACKAGE_PUBLIC_HEADER}")
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

macro(final_deal)
	_mxx_deal_link_packages()
	_mxx_deal_delayload_packages()
	
	string(REPLACE ";" "\n" _project_info "${MXX_PACKAGE_INFO_LIST}")
	configure_file("${CONFIG_TEMPLATE_DIR}/project_info.txt.in" "${CONFIGURE_FILE_DIRECTORY}/project_info.txt")
	unset(MXX_PACKAGE_INFO_LIST CACHE)
endmacro()

# 记录构建配置信息
macro(_record_build_configure)
	set(_mxx_build_cfg_temp "${CONFIG_TEMPLATE_DIR}/wps_build_cfg.ini.in")
	set(_mxx_build_cfg "${CONFIGURE_FILE_DIRECTORY}/wps_build_cfg.ini")
	configure_file("${_mxx_build_cfg_temp}" "${_mxx_build_cfg}")
endmacro()

# 根据当前工作目录配置signfile.yaml
# 在cmake -D 中，应传入变量 SIGNFILE=TRUE
macro(_config_signfile)
	if(SIGNFILE)
		set(sign_file "${CONFIGURE_FILE_DIRECTORY}/signfile.yaml")
		file(TO_NATIVE_PATH "${MXX_OUTPUT_ROOT_DIRECOTRY}" BUILD_OUTPUT_ROOT_PATH)
		configure_file("${CONFIG_TEMPLATE_DIR}/signfile.yaml.in" "${sign_file}")
	endif()
endmacro()

macro(_convert_file_code _src_code _src_file _dst_code _dst_file)
	file(TO_NATIVE_PATH "${_src_file}" _src_file_native)
	file(TO_NATIVE_PATH "${_dst_file}" _dst_file_native)
	execute_process(
		COMMAND "${MXX_ICONV_PATH}" "-f" "${_src_code}" "-t" "${_dst_code}" "${_src_file_native}"
		OUTPUT_FILE "${_dst_file_native}"
		RESULT_VARIABLE _execute_result
	)
	if(NOT _execute_result EQUAL 0)
		message(FATAL_ERROR "iconv.exe convert code failed, file:${_src_file_native}")
	endif()
endmacro()

macro(_mxx_generate_binary_info_file)
	if(NOT "${CURRENT_PACKAGE_TYPE}!" STREQUAL "NONE!" AND
		NOT	"${CURRENT_PACKAGE_TYPE}!" STREQUAL "STATIC!")
		# 在cmake -D时，应包括两个参数:FILE_VERSION PRODUCT_VERSION
		if(MXX_CURRENT_FILE_VERSION)
			set(MXX_FILE_VERSION ${MXX_CURRENT_FILE_VERSION})
		elseif(FILE_VERSION)
			set(MXX_FILE_VERSION ${FILE_VERSION})
		endif()		
		string(REPLACE "." "," MXX_FILE_VERSION ${MXX_FILE_VERSION})
		string(REPLACE "." "," MXX_PRODUCT_VERSION ${PRODUCT_VERSION})
		
		string(TOLOWER ${CURRENT_PACKAGE_NAME} _lower_package_name)
		
		if(MXX_CURRENT_PRODUCT_VERSION)
			set(MXX_PRODUCT_VERSION ${MXX_CURRENT_PRODUCT_VERSION})
		endif()
		
		set(MXX_ORIGINAL_FILE_NAME ${CURRENT_PACKAGE_FILENAME})
		set(MXX_FILE_DESCRIPTION ${MXX_CURRENT_FILE_DESCRIPTION})
		set(MXX_INTERNAL_NAME ${MXX_CURRENT_PACKAGE_NAME})
		set(MXX_COMPANY_NAME ${MXX_CURRENT_COMPANY_NAME})
		set(MXX_COPYRIGHT ${MXX_CURRENT_COPY_RIGHT})
		set(MXX_PRODUCT_NAME ${MXX_CURRENT_PRODUCT_NAME})
		
		set(_rc_file_temp "${CONFIG_TEMPLATE_DIR}/Version.rc.in")
		set(_rc_file "${CMAKE_CURRENT_BINARY_DIR}/Version.rc")
		configure_file(${_rc_file_temp} ${_rc_file}.tmp)
		if("${_rc_file}.tmp" IS_NEWER_THAN "${_rc_file}" AND NOT "!${CMAKE_USE_LTCG}" STREQUAL "!PGOptimize")
			_convert_file_code("UTF-8" "${_rc_file}.tmp" "UTF-16LE" "${_rc_file}")
		endif()
		mxx_add_sources(${_rc_file})
	endif()	
endmacro()

# 给项目增加自定义信息
# mxx_declare_fileinfo([FILE_DESCRIPTION "description"] # 如果不写，则为空
	# [FILE_VERSION "file_version"] # 如果不写，则使用外部传入的文件版本号
	# [PRODUCT_VERSION "product_version"] # 如果不写，则使用外部传入的产品版本号
	# [COMPANY_NAME "company name"] # 默认值"Zhuhai Kingsoft Office Software Co.,Ltd"
	# [PRODUCT_NAME "product name"] # 默认值"DocerSoSo"
	# [COPY_RIGHT "copy right"] # 默认值"Copyright(c)1988-2015 Kingsoft Corporation.  All rights reserved." 
								# 注(c)实为版权符，本文件因编码问题无法显示，具体内容参见文件wps_utf8.cmake 变量WPS_COPY_RIGHT_UTF8
	# )
macro(mxx_declare_fileinfo)
	set(oneValueArgs FILE_VERSION PRODUCT_VERSION
		FILE_DESCRIPTION COMPANY_NAME PRODUCT_NAME COPY_RIGHT NPPLUGIN_MIMETYPE)
	cmake_parse_arguments(info "" "${oneValueArgs}" "" ${ARGN})
	
	if(info_FILE_VERSION)
		set(MXX_CURRENT_FILE_VERSION ${info_FILE_VERSION})
	endif()
	if(info_PRODUCT_VERSION)
		set(MXX_CURRENT_PRODUCT_VERSION ${info_PRODUCT_VERSION})
	endif()
	if(info_FILE_DESCRIPTION)
		set(MXX_CURRENT_FILE_DESCRIPTION "${info_FILE_DESCRIPTION}")
	endif()
	
	if(info_COMPANY_NAME)
		set(MXX_CURRENT_COMPANY_NAME "${info_COMPANY_NAME}")
	else()
		set(MXX_CURRENT_COMPANY_NAME "Zhuhai Kingsoft Office Software Co.,Ltd")
	endif()
	
	if(info_PRODUCT_NAME)
		set(MXX_CURRENT_PRODUCT_NAME "${info_PRODUCT_NAME}")
	else()
		set(MXX_CURRENT_PRODUCT_NAME "${MXX_PRODUCT_NAME}")
	endif()
	
	if(info_COPY_RIGHT)
		set(MXX_CURRENT_COPY_RIGHT "${info_COPY_RIGHT}")
	else()
		set(MXX_CURRENT_COPY_RIGHT "${MXX_COPY_RIGHT_UTF8}")
	endif()
	
	if(info_NPPLUGIN_MIMETYPE)
		set(MXX_NPPLUGIN_MIMETYPE "${info_NPPLUGIN_MIMETYPE}")
	else()
		set(MXX_NPPLUGIN_MIMETYPE "")
	endif()
endmacro()

function(_config_install_info_script)
	file(TO_NATIVE_PATH ${MXX_OUTPUT_ROOT_DIRECOTRY} BUILD_OUTPUT_ROOT_PATH)
	configure_file("${CONFIG_TEMPLATE_DIR}/buildconfig.nsi.in" "${CONFIGURE_FILE_DIRECTORY}/buildconfig.nsi")
	string(REPLACE "." "," _product_version_comma ${PRODUCT_VERSION})
	file(WRITE "${CONFIGURE_FILE_DIRECTORY}/kpacket_buildconfig.txt" "PRODUCTVERSION ${_product_version_comma}")#个人版安装包从此文件读版本号
endfunction()

macro(_config_setupcfg)
	string(REPLACE "." ";" VERSION_LIST "${FILE_VERSION}")
	list(GET VERSION_LIST 0 MAJORVERSION)
	list(GET VERSION_LIST 1 FIRSTVERSION)
	list(GET VERSION_LIST 2 SECONDVERSION)
	list(GET VERSION_LIST 3 DISPLAY_VERSION)
	
	configure_file("${CONFIG_TEMPLATE_DIR}/setup.cfg.in" "${MXX_OUTPUT_ROOT_DIRECOTRY}/bin/cfgs/setup.cfg")
endmacro()

macro(mxx_link_wldap32)
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Wldap32.lib")
endmacro()

macro(mxx_link_Ws2_32)
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Ws2_32.lib")
endmacro()

macro(mxx_link_winmm)
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "winmm.lib")
endmacro()

macro(mxx_link_version)
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "Version.lib")
endmacro()

macro(mxx_link_UxTheme)
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "UxTheme.lib")
endmacro()

macro(mxx_link_wke)
#if("!${CMAKE_BUILD_TYPE}" STREQUAL "!Debug")
#	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "${CMAKE_SOURCE_DIR}/wke/demo/Debug_Cairo_CFLite/libwke/wke.lib")
#elseif("!${CMAKE_BUILD_TYPE}" STREQUAL "!Release")
	list(APPEND CURRENT_PACKAGE_LINK_LIBRARYS "${CMAKE_SOURCE_DIR}/wke/demo/Release_Cairo_CFLite/libwke/wke.lib")
#endif()
endmacro()
