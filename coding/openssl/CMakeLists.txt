if(OS_WIN)
	mxx_extern_package(
		libeay32_md SHARED
		LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/md/lib
		BINARY_NAMES libeay32
		PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/include
		mxx_ADD_INSTALL
		)

	mxx_extern_package(
		ssleay32_md SHARED
		LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/md/lib
		BINARY_NAMES ssleay32
		PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/include
		mxx_ADD_INSTALL
		)

	mxx_extern_package(
		libeay32_mt STATIC
		LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/mt/lib
		BINARY_NAMES libeay32
		PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/include
		)

	mxx_extern_package(
		ssleay32_mt STATIC
		LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/mt/lib
		BINARY_NAMES ssleay32
		PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/openssl-win32-libs/include
		)

	set(OpenSSL libeay32_md ssleay32_md libeay32_mt ssleay32_mt)
	mxx_add_phony_target(OpenSSL ${OpenSSL})

endif()