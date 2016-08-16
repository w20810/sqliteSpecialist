mxx_package(sqliteSpecialist EXECUTABLE)

	mxx_include_directories(
		../../modules/log/src
		../123
		../sqlite/cppsqlite
		../sqlite/sqlite3
		../curl/include/curl
		../uilib
		)
	mxx_add_sources(
	main.cpp
	hdid.cpp
	hdid.h
	CDuiFrame.cpp
	CDuiFrame.h
	stdafx.cpp
	stdafx.h
	targetver.h
	PopWnd.cpp
	PopWnd.h
#	src/duilib.xml
	)
	mxx_link_packages(
#	sqlite3
#	zlibwrap
	libeay32_md
	ssleay32_md
	curls_md
#	json_md
	uilib
#	wke
	sqlite3
	cppsqlite
	)
	mxx_link_version()
#	mxx_link_dbghelper()
	mxx_link_winmm()
	mxx_link_wldap32()
	mxx_link_ws2_32()

mxx_end_package()