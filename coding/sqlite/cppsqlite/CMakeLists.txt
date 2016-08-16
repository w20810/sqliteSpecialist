mxx_package(cppsqlite STATIC)
	mxx_add_definitions(
		_LIB
		)
	mxx_include_directories(
		../sqlite3/
		)
	mxx_add_sources(
		cppsqlite3.cpp
		cppsqlite3.h
		)
mxx_end_package()

mxx_package(cppsqlite_mt STATIC)
	mxx_add_definitions(
		_LIB
		)
	mxx_include_directories(
		../sqlite3/
		)
	mxx_add_sources(
		cppsqlite3.cpp
		cppsqlite3.h
		)
	mxx_set_runtime_library_type(MT)
mxx_end_package()
