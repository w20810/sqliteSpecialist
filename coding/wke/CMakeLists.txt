mxx_extern_package(
	wke_dll MODULE
	LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/demo
	BINARY_NAMES Release_Cairo_CFLite/bin/wke
	#BINARY_NAMES_DEBUG Debug_Cairo_CFLite/bin/wke
	#BINARY_NAMES_RELEASE Release_Cairo_CFLite/bin/wke
	)
	
mxx_extern_package(
	wke STATIC
	LOCATION  ${CMAKE_CURRENT_SOURCE_DIR}/demo/Release_Cairo_CFLite/libwke
	BINARY_NAMES wke
	)