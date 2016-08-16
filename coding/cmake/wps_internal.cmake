include(CMakeParseArguments)
include("${mxx_CMAKE_DIR}/mxx_macros.cmake")

# _verity_pkg_name()
macro(_verity_pkg_name)
	#...
	if(mxx_CURRENT_PACKAGE_TYPE STREQUAL "LNG_LEGACY")
		if(NOT mxx_CURRENT_PACKAGE_NAME MATCHES "^[a-zA-Z0-9]+-[a-zA-Z0-9_]+")
			message(FATAL_ERROR "${mxx_CURRENT_PACKAGE_NAME} is not a valid lng package!")
		endif()
	endif()
endmacro()

function(_get_project_name_and_locale _out_project_name _out_locale)
	string(REGEX REPLACE "-([a-zA-Z0-9]+_[a-zA-Z0-9]+|default)$" "" _project_name "${mxx_CURRENT_PACKAGE_NAME}")
	string(REGEX REPLACE "^[a-zA-Z0-9]+-" "" _locale "${mxx_CURRENT_PACKAGE_NAME}")
	set(${_out_project_name} ${_project_name} PARENT_SCOPE)
	set(${_out_locale} ${_locale} PARENT_SCOPE)
endfunction()

function(_get_definitions _output)
	get_property(_result DIRECTORY PROPERTY COMPILE_DEFINITIONS)
	set(_result ${_result} ${mxx_CURRENT_PACKAGE_DEFINITIONS})
	set(${_output} ${_result} PARENT_SCOPE)
endfunction()

function(_get_compile_args _lang _output)
	if(MSVC)
		set(_def_prefix "/D")
		set(_inc_prefix "/I")
	else()
		set(_def_prefix "-D")
		set(_inc_prefix "-I")
	endif()

	set(_defs)
	foreach(_d ${mxx_CURRENT_PACKAGE_DEFINITIONS})
		string(REPLACE "\"" "\\\"" _d "${_d}")
		list(APPEND _defs "${_def_prefix}${_d}")
	endforeach()
	get_property(_ds DIRECTORY PROPERTY COMPILE_DEFINITIONS)
	foreach(_d ${_ds})
		string(REPLACE "\"" "\\\"" _d "${_d}")
		list(APPEND _defs "${_def_prefix}${_d}")
	endforeach()
	get_property(_is DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
	set(_incs)
	foreach(_i ${_is})
		list(APPEND _incs "${_inc_prefix}\"${_i}\"")
	endforeach()
	
	string(REGEX MATCHALL "[^ ;]+" _flags "${CMAKE_${_lang}_FLAGS} ${CMAKE_${_lang}_FLAGS_${BUILD_TYPE_UPPER}}")
	set(${_output} ${_incs} ${_defs} ${_flags} PARENT_SCOPE)
endfunction()

# dll合并之后，没有一个好的方法将.lib的符号在.dll中导出，windows版本暂时
# 用.def文件来控制导出符号，我们暂时也用这个方法。
macro(_mxx_deal_export_symbols _srcs_ref _gen_ref)
	if(NOT OS_WIN)
		if("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "SHARED!"
			OR "${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "MODULE!")
			set(__srcs ${${_srcs_ref}})
			foreach(_x ${__srcs})
				if(_x MATCHES "\\.def$")
					get_filename_component(_def_file_name ${_x} NAME)
					set(_cpp_file ${CMAKE_CURRENT_BINARY_DIR}/${_def_file_name}.cpp)
					set(_header_file ${CMAKE_CURRENT_SOURCE_DIR}/${_x}.h)
					message(STATUS "${_x} -> ${_def_file_name}.cpp")

					add_custom_command(
						OUTPUT ${_cpp_file}
						COMMAND ${CMAKE_COMMAND} -DDEF_FILE=${CMAKE_CURRENT_SOURCE_DIR}/${_x}
								-DCPP_FILE=${_cpp_file} -DHEADER_FILE=${_header_file}
								-P ${CMAKE_SOURCE_DIR}/cmake/mxx_def2cpp.cmake
						MAIN_DEPENDENCY ${_x}
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						COMMENT "Converting ${_x} to ${_def_file_name}.cpp..."
						)
					set(${_gen_ref} ${_cpp_file} ${_header_file})
					#_mxx_deal_export_symbols_with_def(${_x} ${_gen_ref})
				endif()
			endforeach()
		endif()
	endif()
endmacro()

macro(_mxx_deal_idl _srcs_ref _gen_ref)
	set(__srcs ${${_srcs_ref}})
	foreach(_x ${__srcs})
		if(_x MATCHES "\\.idl$")
			if(OS_WIN)
				mxx_find_midl()
	
				list(REMOVE_ITEM __srcs ${_x})
				get_filename_component(_fname "${_x}" NAME_WE)
				_get_compile_args(IDL _args)
				set(_idl_idl "${CMAKE_CURRENT_SOURCE_DIR}/${_x}")
				set(_pub_dir "${CMAKE_CURRENT_BINARY_DIR}/include/${mxx_CURRENT_PACKAGE_NAME}")
				set(_idl_h "${_pub_dir}/${_fname}.h")
				set(_idl_iid "${_pub_dir}/${_fname}_i.c")
				set(_idl_tlb "${_pub_dir}/${_fname}.tlb")
				set(_idl_dlldata "${_pub_dir}/${_fname}_dlldata.c")
				set(_idl_proxy "${_pub_dir}/${_fname}_p.c")
	
				foreach(_l ${mxx_CURRENT_PACKAGE_INCLUDE_PKG_DIRS})
					list(APPEND _args "/L\"${_l}\"")
				endforeach()
	
				add_custom_command(
					OUTPUT ${_idl_h} ${_idl_iid} ${_idl_tlb}
					COMMAND ${mxx_MIDL} ${_args} /h ${_idl_h} /iid ${_idl_iid} 
							/tlb ${_idl_tlb} /dlldata ${_idl_dlldata} /proxy ${_idl_proxy} ${_idl_idl}
					MAIN_DEPENDENCY ${_x}
					DEPENDS ${_pub_dir} 
					IMPLICIT_DEPENDS CXX ${_x}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
					COMMENT "Compiling ${_x}..."
					)
				list(APPEND ${_gen_ref} ${_idl_h})
				set(mxx_CURRENT_PACKAGE_HAS_PUBLIC_HEADER YES)
			else()
				set(_imm_dir "${CMAKE_SOURCE_DIR}/intermediate/idl/${mxx_CURRENT_PACKAGE_NAME}")
				message(WARNING "Build script could not support idl file."
					" Copy from Coding/intermediate/idl/${mxx_CURRENT_PACKAGE_NAME}")
				get_filename_component(_fname "${_x}" NAME_WE)
				set(_pub_dir "${CMAKE_CURRENT_BINARY_DIR}/include/${mxx_CURRENT_PACKAGE_NAME}")
				set(_idl_h "${_pub_dir}/${_fname}.h")
				set(_idl_iid "${_pub_dir}/${_fname}_i.c")
				add_custom_command(
					OUTPUT ${_idl_h} ${_idl_iid}
					COMMAND mkdir -p ${_pub_dir}
					COMMAND cp ${_imm_dir}/${_fname}.h ${_idl_h}
					COMMAND cp ${_imm_dir}/${_fname}_i.c ${_idl_iid}
					DEPENDS ${_imm_dir}/${_fname}.h ${_imm_dir}/${_fname}_i.c
					)
				list(APPEND ${_gen_ref} ${_idl_h})
				set(mxx_CURRENT_PACKAGE_HAS_PUBLIC_HEADER YES)
			endif()
		endif()
	endforeach()
	set(${_srcs_ref} ${__srcs})
endmacro()

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
			set(_pch_pch ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${mxx_CURRENT_PACKAGE_NAME}.dir/${_pch_header}.pch)
			set(_pch_obj ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${mxx_CURRENT_PACKAGE_NAME}.dir/${_pch_source}.obj)
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
			list(APPEND mxx_CURRENT_PACKAGE_LINK_LIBRARYS "${_pch_obj}")
			add_custom_command(
				OUTPUT ${_pch_pch} ${_pch_obj}
				COMMAND ${CMAKE_CXX_COMPILER} /c ${_args} ${mxx_CURRENT_PACKAGE_OPTIMIZATION} /Yc\"${_pch_header}\" 
						/Fp\"${_pch_pch}\" /Fd\"${_pdb_pos}\" /Fo\"${_pch_obj}\"
						"${CMAKE_CURRENT_SOURCE_DIR}/${_pch_source}"
				MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${_pch_source}
				IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${_pch_source}
				DEPENDS ${${_idl_gen_ref}} # some of pch header depends idl generated file, so...
				COMMENT "Creating Precompiler header..."
				)
		endif()
	else()
		_get_compile_args(CXX _args)
		# pch's source do not used, build same as normal source file.
		list(APPEND _mxx_deal_pch_srcs ${_pch_header} ${_pch_source}) 
		_create_include_ref("${_pch_header}" "${CMAKE_CURRENT_BINARY_DIR}/${_pch_header}")
		set(_pch_pch "${CMAKE_CURRENT_BINARY_DIR}/${_pch_header}.gch")
		foreach(_x ${_mxx_deal_pch_srcs})
			if(_x MATCHES "(\\.cpp$|\\.cc$|\\.cxx$)")
				set_property(SOURCE "${_x}" PROPERTY OBJECT_DEPENDS "${_pch_pch}")
			endif()
		endforeach()
		add_custom_command(
			OUTPUT ${_pch_pch}
			COMMAND ${mxx_COMPILER_LEADER} ${CMAKE_CXX_COMPILER} -x c++-header ${_args} ${mxx_CURRENT_PACKAGE_OPTIMIZATION} "${CMAKE_CURRENT_SOURCE_DIR}/${_pch_header}" -o "${_pch_pch}"
			MAIN_DEPENDENCY ${_pch_header}
			IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${_pch_header}
			DEPENDS ${${_idl_gen_ref}} # some of pch header depends idl generated file, so...
			COMMENT "Creating Precompiler header..."
			)
		if("!${CMAKE_CXX_COMPILER_ID}" STREQUAL "!GNU")
			foreach(_x ${_mxx_deal_pch_srcs})
				if(_x MATCHES "(\\.cpp$|\\.cc$|\\.cxx$)")
					get_property(_flags SOURCE "${_x}" PROPERTY COMPILE_FLAGS)
					set_property(SOURCE "${_x}" 
						PROPERTY COMPILE_FLAGS "${_flags} -fpch-preprocess")
				endif()
			endforeach()
		elseif("!${CMAKE_CXX_COMPILER_ID}" STREQUAL "!Clang")
			foreach(_x ${_mxx_deal_pch_srcs})
				if(_x MATCHES "(\\.cpp$|\\.cc$|\\.cxx$)")
					get_property(_flags SOURCE "${_x}" PROPERTY COMPILE_FLAGS)
					set_property(SOURCE "${_x}" 
						PROPERTY COMPILE_FLAGS "${_flags} -include-pch ${_pch_pch}")
				endif()
			endforeach()
		endif()
	endif()
	set(${_srcs_ref} ${_mxx_deal_pch_srcs})
endmacro()

function(_list_shift_n _list_ref _n _dest_ref)
	set(_result)
	set(_input ${${_list_ref}})
	list(LENGTH _input _input_len)
	while((NOT _n EQUAL 0) AND (NOT _input_len EQUAL 0))
		list(GET _input 0 _ele)
		list(REMOVE_AT _input 0)
		list(APPEND _result ${_ele})
		math(EXPR _n "${_n}-1")
		list(LENGTH _input _input_len)
	endwhile()
	set(${_dest_ref} ${_result} PARENT_SCOPE)
	set(${_list_ref} ${_input} PARENT_SCOPE)
endfunction()

macro(_mxx_deal_automoc _srcs_ref _pch_header)
	mxx_find_qt4_automoc()

	set(_wda_srcs ${${_srcs_ref}})

	_get_definitions(_ds)
	get_property(_is DIRECTORY PROPERTY INCLUDE_DIRECTORIES)

	set(_headers)
	foreach(_f ${_wda_srcs})
		if(_f MATCHES "(\\.c$|\\.cpp$|\\.cc$|\\.cxx$)")
			get_filename_component(_filename ${_f} NAME_WE)
			set(_moc_filename "${CMAKE_CURRENT_BINARY_DIR}/moc/${_filename}.moc")
			add_custom_command(
				OUTPUT ${_moc_filename}
				COMMAND ${CMAKE_COMMAND} "-DDEFINITIONS=\"${_ds}\""
										 "-DINCLUDES=\"${_is}\"" 
										 "-DMOC_BIN=\"${mxx_QT4_MOC}\""
										 "-DINPUT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/${_f}\""
										 "-DOUTPUT_FILE=\"${_moc_filename}\""
										 -P \"${mxx_QT4_AUTOMOC_CXX_SCRIPT}\"
				DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${_f}" "${CMAKE_CURRENT_BINARY_DIR}/moc"
				)
			get_property(_obj_dep SOURCE "${_f}" PROPERTY OBJECT_DEPENDS)
			if(_obj_dep)
				set(_obj_dep "${_obj_dep};")
			endif()
			set_property(SOURCE "${_f}" PROPERTY OBJECT_DEPENDS "${_obj_dep}${_moc_filename}")
			if(NOT MSVC)
				# Makefile无法正确处理OBJECT_DEPENDS?
				list(APPEND _wda_srcs ${_moc_filename})
			endif()
		elseif(_f MATCHES "(\\.h$|\\.hpp$|\\.hh$|\\.hxx$)" AND NOT _f STREQUAL _pch_header)
			list(APPEND _headers "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
		endif()
	endforeach()
	
	if(_headers)
		list(SORT _headers)
		list(REMOVE_DUPLICATES _headers)
	endif()
	
	set(_idx 0)
	while(_headers)
		_list_shift_n(_headers 30 _headers_part)
		set(_list_file_cont "")
		foreach(_h ${_headers_part})
			set(_list_file_cont "${_list_file_cont}${_h}\n")
		endforeach()
		configure_file("${mxx_CONFIG_TEMPLATE_DIR}/moc_list.in" "${CMAKE_CURRENT_BINARY_DIR}/moc/moc_list${_idx}")
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/moc/_automoc${_idx}.cpp"
			COMMAND ${CMAKE_COMMAND} "-DDEFINITIONS=\"${_ds}\""
									 "-DINCLUDES=\"${_is}\"" 
									 "-DMOC_BIN=\"${mxx_QT4_MOC}\""
									 "-DLIST_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/moc/moc_list${_idx}\""
									 "-DOUTPUT_FILE=\"${CMAKE_CURRENT_BINARY_DIR}/moc/_automoc${_idx}.cpp\""
									 "-DPCH_HEADER=\"${_pch_header}\""
									 -P \"${mxx_QT4_AUTOMOC_H_SCRIPT}\"
			DEPENDS ${_headers_part} "${CMAKE_CURRENT_BINARY_DIR}/moc/moc_list${_idx}"
			)		
		list(INSERT _wda_srcs 0 "${CMAKE_CURRENT_BINARY_DIR}/moc/_automoc${_idx}.cpp")#automoc文件较大放在最前 加快并发编译速度
		source_group("Generated Files" FILES "${CMAKE_CURRENT_BINARY_DIR}/moc/_automoc${_idx}.cpp")
		math(EXPR _idx "${_idx}+1")
	endwhile()

	#todo when has multi mxx_add_source with QT4_AUTOMOC 

	set(${_srcs_ref} ${_wda_srcs})
endmacro()

function(_get_compiler_name_with_version _output)
	if(NOT mxx_COMPILER_NAME_WITH_VERSION)
		if(MSVC)
			math(EXPR _majorver "${MSVC_VERSION}/100-6")
			set(mxx_COMPILER_NAME_WITH_VERSION "msvc${_majorver}" CACHE STRING "Compiler name with version.")
		else()
			set(mxx_COMPILER_NAME_WITH_VERSION "gnu" CACHE STRING "Compiler name with version.")
		endif()
	endif()
	set(${_output} ${mxx_COMPILER_NAME_WITH_VERSION} PARENT_SCOPE)
endfunction()

macro(_get_sdk_pkg_name pkg_name _output)
	_get_compiler_name_with_version(_compile)
	string(TOLOWER ${CMAKE_BUILD_TYPE} _build_type)
	set(${_output} "${pkg_name}-${_compile}-${_build_type}.7z")
endmacro()

macro(_mxx_get_pkg_output _outputs)
	set(${_outputs})
	if("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "STATIC!")
		set(_filename "${CMAKE_STATIC_LIBRARY_PREFIX}${mxx_CURRENT_PACKAGE_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}")
		list(APPEND ${_outputs} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_filename}")
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "SHARED!")
		set(_filename "${CMAKE_SHARED_LIBRARY_PREFIX}${mxx_CURRENT_PACKAGE_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		list(APPEND ${_outputs} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_filename}")
		if(WIN32)
			set(_fn "${CMAKE_IMPORT_LIBRARY_PREFIX}${mxx_CURRENT_PACKAGE_NAME}${CMAKE_IMPORT_LIBRARY_SUFFIX}")
			list(APPEND ${_outputs} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_fn}")
		endif()
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "MODULE!")
		set(_filename "${CMAKE_SHARED_MODULE_PREFIX}${mxx_CURRENT_PACKAGE_NAME}${CMAKE_SHARED_MODULE_SUFFIX}")
		list(APPEND ${_outputs} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_filename}")
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "EXECUTABLE!" 
			OR "${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "CONSOLE!")
		list(APPEND ${_outputs} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${mxx_CURRENT_PACKAGE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "LNG_LEGACY!")	
		_get_project_name_and_locale(_prj_name _locale)
		list(APPEND ${_outputs} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/mui/${_locale}/${_prj_name}.lng")
	endif()
	if(mxx_CURRENT_PACKAGE_HAS_PUBLIC_HEADER)
		list(APPEND ${_outputs} "${CMAKE_CURRENT_BINARY_DIR}/include")
	endif()
endmacro()

macro(_mxx_create_sdk_info)
	_get_sdk_pkg_name(${mxx_CURRENT_PACKAGE_NAME} _sdk_filename)
	set(_sdk_full_path "${mxx_SDK_OUTPUT_DIRECTORY}/${_sdk_filename}")
	_mxx_get_pkg_output(_target_output)
	
	if(EXISTS  ${makeSdkFile})
		file(APPEND ${makeSdkFile} "\telseif(\"!\${sdkName}\" STREQUAL \"!${mxx_CURRENT_PACKAGE_NAME}\")\n")
		file(APPEND ${makeSdkFile} "\t\tlist(APPEND sdkInfos ${mxx_CURRENT_PACKAGE_NAME})\n")
		file(APPEND ${makeSdkFile} "\t\tlist(APPEND sdkInfos ${_sdk_full_path})\n")
		foreach(var_target_output ${_target_output})
			file(APPEND ${makeSdkFile} "\t\tlist(APPEND sdkInfos ${var_target_output})\n")
		endforeach()
	endif()
endmacro()

macro(_config_create_api_list)
    file(TO_NATIVE_PATH ${CMAKE_SOURCE_DIR} __CMAKE_SOURCE_DIR)
    
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
		set(_result_dir_ "win32")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
		set(_result_dir_ "win32d")
	endif()
    
    configure_file("${mxx_CONFIG_TEMPLATE_DIR}/create_api_list.bat.in" "${CMAKE_BINARY_DIR}/create_api_list.bat")

endmacro()

macro(_mxx_get_cur_pkg_main_output)
	if("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "SHARED!")
		set(_filename "${CMAKE_SHARED_LIBRARY_PREFIX}${mxx_CURRENT_PACKAGE_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}")
		set(mxx_CURRENT_PACKAGE_FULLPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_filename}")
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "MODULE!")
		set(_filename "${CMAKE_SHARED_MODULE_PREFIX}${mxx_CURRENT_PACKAGE_NAME}${CMAKE_SHARED_MODULE_SUFFIX}")
		set(mxx_CURRENT_PACKAGE_FULLPATH "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_filename}")
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "EXECUTABLE!" 
			OR "${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "CONSOLE!")
		set(mxx_CURRENT_PACKAGE_FULLPATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${mxx_CURRENT_PACKAGE_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
	elseif("${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "LNG_LEGACY!")	
		_get_project_name_and_locale(_prj_name _locale)
		set(mxx_CURRENT_PACKAGE_FULLPATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/mui/${_locale}/${_prj_name}.lng")
	endif()
	
	get_filename_component(mxx_CURRENT_PACKAGE_SUFFIX "${mxx_CURRENT_PACKAGE_FULLPATH}" EXT)
	get_filename_component(mxx_CURRENT_PACKAGE_FILENAME "${mxx_CURRENT_PACKAGE_FULLPATH}" NAME)
endmacro()

macro(_mxx_get_pkg_filename _pkg_name _pkg_type _filename_ref)
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

macro(_mxx_deal_sources_group _srcs_ref _idl_gen_ref)
	set(_wdsg_srcs ${${_srcs_ref}})
	set(_qt4_automoc NO)
	set(_pch_header "")
	set(_pch_source "")
	list(FIND _wdsg_srcs "QT4_AUTOMOC" _automoc_idx)
	if(NOT _automoc_idx EQUAL -1)
		list(REMOVE_AT _wdsg_srcs ${_automoc_idx})
		set(_qt4_automoc YES)
	endif()
	list(FIND _wdsg_srcs "PCH" _pch_idx)
	if(NOT _pch_idx EQUAL -1)
		list(REMOVE_AT _wdsg_srcs ${_pch_idx})
		list(GET _wdsg_srcs ${_pch_idx} _pch_header)
		list(REMOVE_AT _wdsg_srcs ${_pch_idx})
		list(GET _wdsg_srcs ${_pch_idx} _pch_source)
		list(REMOVE_AT _wdsg_srcs ${_pch_idx})
	endif()

	# deal automoc before pch, because of automoc will create source file.
	if(_qt4_automoc)
		_mxx_deal_automoc(_wdsg_srcs "${_pch_header}")
	endif()

	if(_pch_header)
		_mxx_deal_pch(_wdsg_srcs ${_pch_header} ${_pch_source} ${_idl_gen_ref})
	endif()	

	set(${_srcs_ref} ${_wdsg_srcs})
endmacro()

macro(_mxx_deal_uic _srcs_ref _gen_ref)
	set(__srcs ${${_srcs_ref}})
	foreach(_x ${__srcs})
		if(_x MATCHES "\\.ui$")
			mxx_find_qt4_uic()
			get_filename_component(_fname "${_x}" NAME_WE)
			set(_out_name "ui_${_fname}.h")
			set(_out_fullname "${CMAKE_CURRENT_BINARY_DIR}/uic/${_out_name}")

			add_custom_command(
				OUTPUT ${_out_fullname}
				COMMAND ${mxx_QT4_UIC} -o ${_out_fullname} "${CMAKE_CURRENT_SOURCE_DIR}/${_x}"
				MAIN_DEPENDENCY ${_x}
				DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/uic"
				COMMENT "Compiling ${_x}..."
				)
			list(APPEND ${_gen_ref} ${_out_fullname})
		endif()
	endforeach()
	source_group("Generated Files" FILES "${CMAKE_CURRENT_BINARY_DIR}/uic/${_out_name}")
endmacro()

macro(_mxx_deal_qrc _srcs_ref _gen_ref)
	set(__srcs ${${_srcs_ref}})
	set(_qrc_list)
	foreach(_x ${__srcs})
		if(_x MATCHES "\\.qrc$")
			mxx_find_qt4_rcc()
			_mxx_scan_qrc(${_x} __srcs _qresource_list)
			list(APPEND _qrc_list ${_x})
			get_filename_component(_fname "${_x}" NAME_WE)
			set(_out_name "qrc_${_fname}.cpp")
			set(_out_fullname "${CMAKE_CURRENT_BINARY_DIR}/moc/${_out_name}")
			add_custom_command(
				OUTPUT ${_out_fullname}
				COMMAND ${mxx_QT4_RCC} -name "${_fname}" -no-compress "${CMAKE_CURRENT_SOURCE_DIR}/${_x}" -o "${_out_fullname}"
				MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${_x}"
				DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/moc" ${_qresource_list}
				COMMENT "Compiling ${_x}..."
				)
			list(APPEND ${_gen_ref} ${_out_fullname})
			source_group("Generated Files" FILES "${_out_fullname}")
		endif()
	endforeach()
	if(_qrc_list)
		source_group(qrc FILES ${_qrc_list})
	endif()
	set(${_srcs_ref} ${__srcs})
endmacro()

function(_mxx_get_ts_language _ts_fpath _out_lang)
	file(STRINGS ${_ts_fpath} _lines LIMIT_COUNT 10)
	foreach(_line ${_lines})
		if(_line MATCHES "TS.*language=\".*\"")
			string(REGEX REPLACE ".*TS.*language=\"(.*)\".*" "\\1" _lang ${_line})
			set(${_out_lang} ${_lang} PARENT_SCOPE)
			return()
		endif()
	endforeach()
	message(FATAL_ERROR "Cannot get language from ${_ts_fpath}")
endfunction()

macro(_mxx_deal_ts _srcs_ref _gen_ref)
	set(__srcs ${${_srcs_ref}})
	set(_ts_list)
	foreach(_x ${__srcs})
		if(_x MATCHES "\\.ts$")
			list(APPEND _ts_list ${_x})
		endif()
	endforeach()
	if(_ts_list)
		mxx_find_qt4_lrelease()
		mxx_find_qt4_rcc()
		
		set(_ts_langs)
		foreach(_x ${_ts_list})
			_mxx_get_ts_language(${_x} _lang)
			set(_ts_langs ${_ts_langs} ${_lang})
			set(_ts_${_lang}_files ${_ts_${_lang}_files} ${_x})
		endforeach()
		list(REMOVE_DUPLICATES _ts_langs)
		
		set(_qm_files)
		set(_qm_fpathes)
		foreach(_lang ${_ts_langs})
			set(_files ${_ts_${_lang}_files})
			set(_qm_file "${CMAKE_CURRENT_BINARY_DIR}/qm/${_lang}.qm")
			list(GET _files 0 _main_dep)
			add_custom_command(
				OUTPUT "${_qm_file}"
				COMMAND ${mxx_QT4_LRELEASE} ${_files} -qm "${_qm_file}"
				MAIN_DEPENDENCY ${_main_dep}
				DEPENDS ${_files} ${CMAKE_CURRENT_BINARY_DIR}/qm
				COMMENT "${_files} => ${_qm_file}"
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
				)
			list(APPEND _qm_files "qm/${_lang}.qm")
			list(APPEND _qm_fpathes ${_qm_file})
		endforeach()
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/qm"
			COMMAND "${CMAKE_COMMAND}" -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/qm
			)
		
		set(_qrc_cont)
		set(_qm_qrc "${CMAKE_CURRENT_BINARY_DIR}/qm.qrc")
		foreach(_qm ${_qm_files})
			set(_qrc_cont "${_qrc_cont}\t\t<file>${_qm}</file>\n")
		endforeach()
		set(QRC_CONT "${_qrc_cont}")
		configure_file("${mxx_CONFIG_TEMPLATE_DIR}/qm.qrc.in" ${_qm_qrc})
		
		set(_qm_cpp "${CMAKE_CURRENT_BINARY_DIR}/qm/qrc_qm.cpp")
		add_custom_command(
			OUTPUT ${_qm_cpp}
			COMMAND ${mxx_QT4_RCC} ${_qm_qrc} -o ${_qm_cpp}
			MAIN_DEPENDENCY ${_qm_qrc}
			DEPENDS ${_qm_fpathes}
			COMMENT "Generate ${_qm_cpp}..."
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
			)
		source_group("Generated Files" FILES ${_qm_cpp})
		list(APPEND ${_gen_ref} ${_qm_cpp})
	endif()
endmacro()

macro(_mxx_scan_qrc _qrc_file _wsq_srcs _qresource_list_ref)
	#如下configure_file 是为保证qrc文件有改动时cmake会重新配置
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/${_qrc_file}" "${CMAKE_CURRENT_BINARY_DIR}/${_qrc_file}" COPYONLY)
	set(_qrc_text)
	set(_wds_qres_list)
	get_filename_component(_fname "${_qrc_file}" NAME_WE)
	list(FIND ${_wsq_srcs} "${_qrc_file}" _qrc_file_index)
	get_filename_component(_qrc_path "${_qrc_file}" PATH)
	
	file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/${_qrc_file}" _qrc_text REGEX "<file")
	#资源文件名不必含“]” 所以先注释掉 如果使用如下代码 还需在添加文件到项目前加上“?”改回“]”的代码
	#foreach(_qrc_str ${_qrc_text})#cmake list有bug 列表项中如果含有“]” 之后的内容则会全部作为一项处理 所以先替换为“?”加入时再改回（http://public.kitware.com/Bug/view.php?id=9317）
		#string(REPLACE "]" "?" _qresource_file ${_qrc_str})
		#list(APPEND _wds_qres_list ${_qresource_file})
	#endforeach()
	#set(_qrc_text ${_wds_qres_list})
	
	set(_wds_qres_list)
	foreach(_qrc_str ${_qrc_text})
		string(REGEX REPLACE "[^<]*<file.*>(.*)</file>.*" "\\1" _qresource_file ${_qrc_str})#VS项目中的文件没有别名 所以不处理alias属性
		list(APPEND _wds_qres_list "${CMAKE_CURRENT_SOURCE_DIR}/${_qrc_path}/${_qresource_file}")
	endforeach()
	list(REMOVE_DUPLICATES _wds_qres_list)
	if(NOT _qrc_file_index EQUAL "-1")
	list(INSERT ${_wsq_srcs} ${_qrc_file_index} ${_wds_qres_list})
	source_group("qrc\\${_fname}" FILES ${_wds_qres_list})
	endif()
	set(${_qresource_list_ref} ${_wds_qres_list})
endmacro()

macro(_mxx_deal_sources _srcs_ref)
	set(_wds_srcs ${${_srcs_ref}})
	set(_wds_idl_gen)

	_mxx_deal_export_symbols(_wds_srcs _wds_export_symbols_gen)

	_mxx_deal_idl(_wds_srcs _wds_idl_gen)
	if(_wds_idl_gen)
		set(_pub_dir "${CMAKE_CURRENT_BINARY_DIR}/include")
		set(_pub_dir2 "${_pub_dir}/${mxx_CURRENT_PACKAGE_NAME}")
		mxx_include_directories(${_pub_dir2} ${_pub_dir})
		add_custom_command(
			OUTPUT ${_pub_dir}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${_pub_dir}
			)
		add_custom_command(
			OUTPUT ${_pub_dir2}
			COMMAND ${CMAKE_COMMAND} -E make_directory ${_pub_dir2}
			DEPENDS ${_pub_dir}
			)
	endif()

	set(_wds_uic_gen)
	_mxx_deal_uic(_wds_srcs _wds_uic_gen)
	if(_wds_uic_gen)
		mxx_include_directories("${CMAKE_CURRENT_BINARY_DIR}/uic")
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/uic"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/uic"
			)
	endif()
	
	set(_wds_qrc_gen)
	_mxx_deal_qrc(_wds_srcs _wds_qrc_gen)
	if(_wds_qrc_gen)
		mxx_include_directories("${CMAKE_CURRENT_BINARY_DIR}/moc")
		add_custom_command(
			OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/moc"
			COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/moc"
			)
	endif()

	set(_wds_ts_gen)
	_mxx_deal_ts(_wds_srcs _wds_ts_gen)

	list(FIND _wds_srcs "QT4_AUTOMOC" _automoc_idx)
	if(NOT _automoc_idx EQUAL -1)
		mxx_include_directories("${CMAKE_CURRENT_BINARY_DIR}/moc")
	endif()

	set(_srcs_for_compile)
	set(_srcs_grouped)
	foreach(_src ${_wds_srcs})
		if(NOT _src STREQUAL "mxx_SOURCE_SEPARATOR")
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

macro(_mxx_get_target_implib _gti_pkg _proj _output_ref)
	if(TARGET ${_gti_pkg})
		get_property(_pkg_type TARGET ${_gti_pkg} PROPERTY mxx_PACKAGE_TYPE)
		if(NOT _pkg_type)
			message(FATAL_ERROR "${_gti_pkg} is not a valid mxx source package.Error in ${_proj}")
		endif()
		if("${_pkg_type}!" STREQUAL "STATIC!")
			if(MSVC)
				set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_gti_pkg}.lib")
			else()
				set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/lib${_gti_pkg}.a")
			endif()
		elseif("${_pkg_type}!" STREQUAL "SHARED!")
			if(MSVC)
				set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_gti_pkg}.lib")
			elseif(OS_LINUX OR OS_BSD)
				set(${_output_ref} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${_gti_pkg}.so")
			elseif(OS_MAC)
				set(${_output_ref} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${_gti_pkg}.dylib")
			else()
				set(${_output_ref} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${_gti_pkg}.so")
			endif()
		elseif("${_pkg_type}!" STREQUAL "EXECUTABLE!")
			if(MSVC)
				set(${_output_ref} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_gti_pkg}.lib")
			elseif(OS_LINUX OR OS_BSD)
				set(${_output_ref} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_gti_pkg}")
			elseif(OS_MAC)
				set(${_output_ref} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_gti_pkg}.app/Contents/MacOS/${_gti_pkg}")
			else()
				set(${_output_ref} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_gti_pkg}")
			endif()
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

macro(_mxx_deal_link_packages_internal _pkg)
	set(_link_libs)
	get_property(_pkg_compile_flags TARGET ${_pkg} PROPERTY COMPILE_FLAGS)
	string(REGEX MATCH "/M[DT]+" _pkg_runtime_lib_type "${_pkg_compile_flags}")
	foreach(_d_pkg ${ARGN})
		if (NOT TARGET ${_d_pkg})
			message(FATAL_ERROR "package ${_d_pkg} not exist. referenced by ${_pkg}")
		endif()
		get_property(_d_pkg_compile_flags TARGET ${_d_pkg} PROPERTY COMPILE_FLAGS)
		string(REGEX MATCH "/M[DT]+" _d_pkg_runtime_lib_type "${_d_pkg_compile_flags}")
		get_property(_d_pkg_type TARGET ${_d_pkg} PROPERTY mxx_PACKAGE_TYPE)
		if(_d_pkg_runtime_lib_type AND "!${_d_pkg_type}" STREQUAL "!STATIC" AND NOT "!${_pkg_runtime_lib_type}" STREQUAL "!${_d_pkg_runtime_lib_type}")
			#检查package与被link package运行时库类型是否相同 无权限的项目不会被检查
			message(FATAL_ERROR "package ${_pkg} is ${_pkg_runtime_lib_type}, but ${_d_pkg} is ${_d_pkg_runtime_lib_type}.")
		endif()
		_mxx_get_target_implib(${_d_pkg} ${_pkg} _implib)
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

macro(_mxx_deal_resource_file _res_ref _res_path_list_ref _res_verify_cmd _res_setup_cmd)
	if(NOT "!${_res_setup_cmd}" STREQUAL "!" AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
		list(FIND mxx_SETUP_CMD_LIST "${_res_setup_cmd}" _res_setup_index)
		if(_res_setup_index EQUAL -1)
			message(FATAL_ERROR "${_res_setup_cmd} is not a supported command.\nsupport commands: ${mxx_SETUP_CMD_LIST}.")	
		endif()
		set(_res_command ${_res_setup_cmd})
	else()
		set(_res_command ${CMAKE_COMMAND} -E copy)
	endif()
	if(NOT "!${_res_verify_cmd}" STREQUAL "!")
		list(FIND mxx_VERIFY_CMD_LIST "${_res_verify_cmd}" _res_verify_index)
		if(_res_verify_index EQUAL -1)
			message(FATAL_ERROR "${_res_verify_cmd} is not a supported command.\nsupport commands: ${mxx_VERIFY_CMD_LIST}.")
		endif()
	endif()
	set(_res_list ${${_res_ref}})
	set(_res_gen)
	foreach(item ${_res_list})
		set(_res_verify_command)
		if(NOT "!${_res_verify_cmd}" STREQUAL "!")
			if(mxx_DAILY_BUILD_MODE)
				set(_res_verify_command ${_res_verify_cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${item} --output-warning)
			else()
				set(_res_verify_command ${_res_verify_cmd} ${CMAKE_CURRENT_SOURCE_DIR}/${item} --output-error)
			endif()
		endif()
		set(_src_file ${CMAKE_CURRENT_SOURCE_DIR}/${item})
		set(_dest_file ${mxx_CURRENT_RESES_GROUP_DESTPATH}/${item})
		if(OS_UNIX)# cmake bugs about ' ( ) under linux
			string(REPLACE "'" "\\'" _src_file ${_src_file})
			string(REPLACE "'" "\\'" _dest_file ${_dest_file})
			string(REPLACE "(" "\\(" _src_file ${_src_file})
			string(REPLACE "(" "\\(" _dest_file ${_dest_file})
			string(REPLACE ")" "\\)" _src_file ${_src_file})
			string(REPLACE ")" "\\)" _dest_file ${_dest_file})
		endif()
        
        set(_cur_command ${_res_command})
        if(IS_DIRECTORY ${_src_file})
            set(_cur_command ${CMAKE_COMMAND} -E copy_directory)
        endif()
        
		add_custom_command(
			OUTPUT ${mxx_CURRENT_RESES_GROUP_DESTPATH}/${item}
			COMMAND ${_res_verify_command}
			COMMAND ${_cur_command} ${_src_file} ${_dest_file}
			MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${item}
			)
		list(APPEND _res_gen ${mxx_CURRENT_RESES_GROUP_DESTPATH}/${item})
	endforeach()
	source_group(res FILES ${_res_list})
	set(${_res_path_list_ref} ${_res_gen})
endmacro()

macro(_mxx_gen_res_stamp _res_ref _res_stamp)
	set(_res_list ${${_res_ref}})
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
		COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/res.stamp
		DEPENDS ${_res_list}
		)
	source_group(res\\stamp FILES ${CMAKE_CURRENT_BINARY_DIR}/res.stamp)
	set(${_res_stamp} ${CMAKE_CURRENT_BINARY_DIR}/res.stamp)
endmacro()

# 解压缩sdk包
macro(_decompression_sdk _pkg_name _pkg_sdk_file)
	set(_pkg_depend_targets ${ARGN})
	mxx_find_7z()
	set(_is_pkg_type_shared FALSE)
	if(OS_WIN)
		set(_pkg_file_dll "${_pkg_name}.dll")
		set(_pkg_file_lib "${_pkg_name}.lib")
	elseif(OS_LINUX)
		set(_pkg_file_dll "lib${_pkg_name}.so")
		set(_pkg_file_lib "lib${_pkg_name}.a")
	else()
		message(FATAL_ERROR "Do Not Support for CURRENT platform.....")
	endif()
	execute_process(
		COMMAND ${mxx_7Z} l "${_pkg_sdk_file}"
		OUTPUT_VARIABLE _sdk_list_output
	)
	if(_sdk_list_output MATCHES ${_pkg_file_dll})
		set(_is_pkg_type_shared TRUE)#通过sdk里的文件判断项目类型
	endif()
	
	set(_unzip_args_dll "e" "${_pkg_sdk_file}" "-y" "-r" "-aoa" "-o${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "${_pkg_file_dll}")
	set(_unzip_args_lib "e" "${_pkg_sdk_file}" "-y" "-r" "-aoa" "-o${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}" "${_pkg_file_lib}")
	set(_unzip_args_include "e" "${_pkg_sdk_file}" "-y" "-r" "-aoa" "-o${CMAKE_CURRENT_BINARY_DIR}/include" "include/*.*")
	
	if(OS_WIN OR NOT _is_pkg_type_shared)#windows sdk一定会有lib文件
		# TODO:暂时对于include文件夹中的文件无法使用touch命令
		add_custom_command(
			OUTPUT "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_pkg_file_lib}"
			COMMAND ${mxx_7Z} ${_unzip_args_lib}
			COMMAND ${mxx_7Z} ${_unzip_args_include}
			COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_pkg_file_lib}"
			MAIN_DEPENDENCY ${_pkg_sdk_file}
			COMMENT "Use ${_pkg_sdk_file}!"
		)
		set(_pkg_depend_files "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${_pkg_file_lib}")
	endif()

	if(_is_pkg_type_shared)
		add_custom_command(
			OUTPUT "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_pkg_file_dll}"
			COMMAND ${mxx_7Z} ${_unzip_args_dll}
			COMMAND ${CMAKE_COMMAND} -E touch "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_pkg_file_dll}"
			MAIN_DEPENDENCY ${_pkg_sdk_file}
			COMMENT "Use ${_pkg_sdk_file}!"
		)
		set(_pkg_depend_files ${_pkg_depend_files} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${_pkg_file_dll}")
	endif()
	add_custom_target(${_pkg_name} ALL  #TODO: 部分受限项目不被依赖但要被解压 所以临时加ALL
			DEPENDS ${_pkg_depend_files}
			COMMENT "custom use sdk: ${_pkg_sdk_file}"
		)
	if(_is_pkg_type_shared)
		set_property(TARGET ${_pkg_name} PROPERTY mxx_PACKAGE_TYPE "SHARED")
	else()
		set_property(TARGET ${_pkg_name} PROPERTY mxx_PACKAGE_TYPE "STATIC")
	endif()
	if(_pkg_depend_targets)
		add_dependencies(${_pkg_name} ${_pkg_depend_targets})
	endif()
endmacro()

macro(_mxx_generate_binary_info_file)
	if(NOT "${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "NONE!" AND
		NOT	"${mxx_CURRENT_PACKAGE_TYPE}!" STREQUAL "STATIC!")
		# 在cmake -D时，应包括两个参数:FILE_VERSION PRODUCT_VERSION
		if(mxx_CURRENT_FILE_VERSION)
			set(mxx_FILE_VERSION ${mxx_CURRENT_FILE_VERSION})
		elseif(FILE_VERSION)
			set(mxx_FILE_VERSION ${FILE_VERSION})
		endif()		
		string(REPLACE "." "," mxx_FILE_VERSION ${mxx_FILE_VERSION})
		string(REPLACE "." "," mxx_PRODUCT_VERSION ${PRODUCT_VERSION})
		
		string(TOLOWER ${mxx_CURRENT_PACKAGE_NAME} _lower_package_name)
		
		# bug:175787;175788:三个项目主版本号改成 11
		if(("!${_lower_package_name}" STREQUAL "!mxx") OR ("!${_lower_package_name}" STREQUAL "!et") OR "!${_lower_package_name}" STREQUAL "!wpp")
			string(REGEX REPLACE "^[0-9]+" "11" mxx_FILE_VERSION ${mxx_FILE_VERSION})
			string(REGEX REPLACE "^[0-9]+" "11" mxx_PRODUCT_VERSION ${mxx_PRODUCT_VERSION})
		endif()
		
		if(mxx_CURRENT_PRODUCT_VERSION)
			set(mxx_PRODUCT_VERSION ${mxx_CURRENT_PRODUCT_VERSION})
		endif()
		
		set(mxx_ORIGINAL_FILE_NAME ${mxx_CURRENT_PACKAGE_FILENAME})
		set(mxx_FILE_DESCRIPTION ${mxx_CURRENT_FILE_DESCRIPTION})
		set(mxx_INTERNAL_NAME ${mxx_CURRENT_PACKAGE_NAME})
		set(mxx_COMPANY_NAME ${mxx_CURRENT_COMPANY_NAME})
		set(mxx_COPYRIGHT ${mxx_CURRENT_COPY_RIGHT})
		set(mxx_PRODUCT_NAME ${mxx_CURRENT_PRODUCT_NAME})
		
		set(_rc_file_temp "${mxx_CONFIG_TEMPLATE_DIR}/Version.rc.in")
		set(_rc_file "${CMAKE_CURRENT_BINARY_DIR}/${mxx_CURRENT_PACKAGE_NAME}Version.rc")
		configure_file(${_rc_file_temp} ${_rc_file}.tmp)
		if("${_rc_file}.tmp" IS_NEWER_THAN "${_rc_file}" AND NOT "!${CMAKE_USE_LTCG}" STREQUAL "!PGOptimize")
			_convert_file_code("UTF-8" "${_rc_file}.tmp" "UTF-16LE" "${_rc_file}")
		endif()
		mxx_add_sources(${_rc_file})
	endif()	
endmacro()

# 根据当前工作目录配置signfile.yaml
# 在cmake -D 中，应传入变量 SIGNFILE=TRUE
macro(_config_signfile)
	if(SIGNFILE)
		set(sign_file "${mxx_CONFIGURE_FILE_DIRECTORY}/signfile.yaml")
		file(TO_NATIVE_PATH "${mxx_OUTPUT_ROOT_DIRECOTRY}" BUILD_OUTPUT_ROOT_PATH)
		configure_file("${mxx_CONFIG_TEMPLATE_DIR}/signfile.yaml.in" "${sign_file}")
	endif()
endmacro()

function(_config_install_info_script)
	file(TO_NATIVE_PATH ${mxx_OUTPUT_ROOT_DIRECOTRY} BUILD_OUTPUT_ROOT_PATH)
	configure_file("${mxx_CONFIG_TEMPLATE_DIR}/buildconfig.nsi.in" "${mxx_CONFIGURE_FILE_DIRECTORY}/buildconfig.nsi")
	string(REPLACE "." "," _product_version_comma ${PRODUCT_VERSION})
	file(WRITE "${mxx_CONFIGURE_FILE_DIRECTORY}/kpacket_buildconfig.txt" "PRODUCTVERSION ${_product_version_comma}")#个人版安装包从此文件读版本号
endfunction()

# 记录构建配置信息
macro(_record_build_configure)
	set(_mxx_build_cfg_temp "${mxx_CONFIG_TEMPLATE_DIR}/mxx_build_cfg.ini.in")
	set(_mxx_build_cfg "${mxx_CONFIGURE_FILE_DIRECTORY}/mxx_build_cfg.ini")
	configure_file("${_mxx_build_cfg_temp}" "${_mxx_build_cfg}")
endmacro()

macro(_convert_file_code _src_code _src_file _dst_code _dst_file)
	file(TO_NATIVE_PATH "${_src_file}" _src_file_native)
	file(TO_NATIVE_PATH "${_dst_file}" _dst_file_native)
	execute_process(
		COMMAND "${mxx_ICONV_PATH}" "-f" "${_src_code}" "-t" "${_dst_code}" "${_src_file_native}"
		OUTPUT_FILE "${_dst_file_native}"
		RESULT_VARIABLE _execute_result
	)
	if(NOT _execute_result EQUAL 0)
		message(FATAL_ERROR "iconv.exe convert code failed, file:${_src_file_native}")
	endif()
endmacro()

macro(_config_whatsnew)
	file(GLOB _whatsnew_in_list "${mxx_CONFIG_TEMPLATE_DIR}/WhatsnewFiles/*.txt.in")
	foreach(_whatsnew_in ${_whatsnew_in_list})
		get_filename_component(_file_name ${_whatsnew_in} NAME_WE)
		configure_file("${_whatsnew_in}" "${mxx_OUTPUT_ROOT_DIRECOTRY}/WhatsnewFiles/${_file_name}.txt" @ONLY)
	endforeach()
endmacro()

macro(_append_target_property_string _target _property _val)
	get_target_property(_orig_val ${_target} ${_property})
	if(_orig_val)
		set_target_properties(${_target} PROPERTIES ${_property} "${_orig_val} ${_val}")
	else()
		set_target_properties(${_target} PROPERTIES ${_property} "${_val}")
	endif()
endmacro()

macro(_args_system_filter _filtered_args_ref)
	set(${_filtered_args_ref})
	_impl_args_system_filter(${_filtered_args_ref} ${ARGN})
endmacro()

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

macro(_create_include_ref _f _header)
	if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
		message(FATAL_ERROR "${_f} does not exists!")
	endif()
	if(NOT EXISTS "${_header}")
		get_filename_component(_pub_d "${_header}" PATH)
		file(RELATIVE_PATH _relative_path "${_pub_d}" "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
		set(_cont "// file created by mxx build system, do not edit!\n\n#include \"${_relative_path}\"\n\n")
		file(WRITE "${_header}" "${_cont}")
	endif()
endmacro()

macro(_create_public_header_ref _f)
	set(_pub_h "${CMAKE_BINARY_DIR}/public_header/${mxx_CURRENT_PACKAGE_NAME}/${_f}")
	_create_include_ref("${_f}" "${_pub_h}")
endmacro()

macro(_create_include_ref _f)
	if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
		message(FATAL_ERROR "${_f} does not exists!")
	endif()
	set(_pub_h "${CMAKE_BINARY_DIR}/public_header/${mxx_CURRENT_PACKAGE_NAME}/${_f}")
	if(NOT EXISTS "${_pub_h}")
		get_filename_component(_pub_d "${_pub_h}" PATH)
		file(RELATIVE_PATH _relative_path "${_pub_d}" "${CMAKE_CURRENT_SOURCE_DIR}/${_f}")
		set(_cont "// file created by mxx build system, do not edit!\n\n#include \"${_relative_path}\"\n\n")
		file(WRITE "${_pub_h}" "${_cont}")
	endif()
endmacro()
macro(_disable_analysis_for_generated_file _srcs_ref)
	set(_dafgf_srcs ${${_srcs_ref}})
	if(mxx_STATIC_CODE_ANALYZE)
		foreach(_src ${_dafgf_srcs})
			if((IS_ABSOLUTE ${_src}) AND 
				(NOT _src MATCHES ${CMAKE_SOURCE_DIR}) AND 
				_src MATCHES "/qrc_.*\\.cpp$"
				) #禁止分析所有生成的代码，Coding中的警告也会减少，先只禁止qrc生成的cpp文件如qrc_icons.cpp保证可以正常编过即可
				set_property(SOURCE "${_src}" PROPERTY COMPILE_FLAGS "/analyze-")
			endif()
		endforeach()
	endif()
endmacro()

macro(_get_mxx_branch _var_ref)
	find_program(SVN_PROGRAM svn)
	if(NOT SVN_PROGRAM)
		message(FATAL_ERROR "Cannot found svn in your %PATH%")
	endif()
	execute_process(COMMAND "${SVN_PROGRAM}" info "${CMAKE_SOURCE_DIR}"
					OUTPUT_VARIABLE _svn_info)
	string(REGEX REPLACE ".*branches/(.*)/Coding.*" "\\1" _branch "${_svn_info}")
	if (NOT _branch)
		string(REGEX REPLACE ".*(trunk)/Coding.*" "\\1" _branch "${_svn_info}")
	endif()
	if (NOT _branch)
		message(FATAL_ERROR "Cannot get branch name from svn")
	endif()
	set(${_var_ref} "${_branch}")
endmacro()

macro(_gen_irm_manifest) 
	set(mxx_IRM_TOOLS_DIR ${mxx_CMAKE_DIR}/bin/win32/irmtools)
	set(mxx_IRM_PRIVATE_DIR ${CMAKE_SOURCE_DIR}/../Environ/DailyBuilder/SVRBIN/private)
	if(EXISTS ${mxx_IRM_TOOLS_DIR} AND EXISTS ${mxx_IRM_PRIVATE_DIR})
		configure_file("${mxx_IRM_TOOLS_DIR}/mxxirm.mcf.in" "${CMAKE_BINARY_DIR}/mxx_configure_file/mxxirm.mcf")
		configure_file("${mxx_IRM_TOOLS_DIR}/etirm.mcf.in" "${CMAKE_BINARY_DIR}/mxx_configure_file/etirm.mcf")
		configure_file("${mxx_CONFIG_TEMPLATE_DIR}/GenIrmManifest.bat.in" "${CMAKE_BINARY_DIR}/GenIrmManifest.bat")
	endif()
endmacro()
