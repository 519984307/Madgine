
if (NOT COMMAND include_guard)

	macro(include_guard)

		set(__filename "${CMAKE_CURRENT_LIST_FILE}")
		get_property(already_included GLOBAL PROPERTY "pr_${__filename}")
		if(already_included)
			return()
		endif()
		set_property(GLOBAL PROPERTY "pr_${__filename}" TRUE)

	endmacro(include_guard)

endif()

if (NOT COMMAND once)

	macro(once)
		include_guard(GLOBAL)
	endmacro(once)

endif()

once()

  
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	add_compile_options(-Wall -fpermissive)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
endif ()

if (NOT MSVC)
	add_compile_options(-fvisibility=hidden)
endif()
