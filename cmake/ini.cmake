include(Util)

once()


function (read_ini_file filepath prefix)

	MESSAGE(STATUS "Reading ini file ${filepath} ...")

    file( STRINGS ${filepath} content )
	set(current_group "")

	set(${prefix}_GROUPS PARENT_SCOPE)

    foreach(LINE ${content})
        if("${LINE}" MATCHES "\\[(.*)\\]")
			set(current_group ${CMAKE_MATCH_1})
			set(${prefix}_GROUPS ${${prefix}_GROUPS} ${current_group} PARENT_SCOPE)
		elseif("${LINE}" MATCHES "(.*)=(.*)")
			if (NOT current_group)
				MESSAGE(SEND_ERROR "Field without group in Ini-file: ${filepath}!")
			endif()
			set(${prefix}_${current_group}_${CMAKE_MATCH_1} ${CMAKE_MATCH_2} PARENT_SCOPE)
			#MESSAGE(STATUS "Setting ${prefix}_${current_group}_${CMAKE_MATCH_1} to ${CMAKE_MATCH_2}")
		endif()

    endforeach()

endfunction (read_ini_file)