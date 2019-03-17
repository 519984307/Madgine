include(Util)

include_guard()

include (Workspace)

include (Plugins)

if (ANDROID)

	set (Android_List_dir ${CMAKE_CURRENT_LIST_DIR})

	if (NOT ANDROID_SDK)
		MESSAGE(SEND_ERROR "No ANDROID_SDK location provided!")
	endif()

	MESSAGE(STATUS "Targeting Android-SDK version: ${ANDROID_PLATFORM_LEVEL}")


	file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/gradle)

	if (CMAKE_HOST_WIN32)	
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gradle/gradlew
			COMMAND gradle wrapper --gradle-version=4.10.2 --distribution-type=all
			COMMAND gradle --stop
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gradle)
	else()
		add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/gradle/gradlew
			COMMAND gradle wrapper --gradle-version=4.10.2 --distribution-type=all			
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/gradle)
	endif()

	add_custom_target(gradlew DEPENDS ${CMAKE_BINARY_DIR}/gradle/gradlew)

	macro(add_workspace_executable target)

		set(target ${target})

		string(REGEX REPLACE "\\\\" "\\\\\\\\" ANDROID_SDK_ESCAPED "${ANDROID_SDK}")

		configure_file(${Android_List_dir}/android/build.gradle.in build.gradle @ONLY)
		configure_file(${Android_List_dir}/android/local.properties.in local.properties @ONLY)
		configure_file(${Android_List_dir}/android/AndroidManifest.xml.in AndroidManifest.xml @ONLY)
		#configure_file(${Android_List_dir}/android/launch.vs.json.in ${CMAKE_SOURCE_DIR}/.vs/launch.vs.json @ONLY)
		#configure_file(${Android_List_dir}/android/debug_apk.bat.in debug_${target}_apk.bat @ONLY)
		#configure_file(${Android_List_dir}/android/launch_apk.sh.in launch_${target}_apk.sh @ONLY)
		#configure_file(${Android_List_dir}/android/gdbcommands.in gdbcommands_${target}_apk @ONLY)
		#configure_file(${Android_List_dir}/android/launch_jdb.bat.in launch_jdb.bat @ONLY)

		add_library(${target} SHARED ${ARGN})

		if (STATIC_BUILD)
			patch_toplevel_target(${target})
		endif()

		_add_workspace_executable(${target}_apk android/dummy.cpp)

		target_link_libraries(${target}_apk PRIVATE ${target})
	
		if (CMAKE_HOST_WIN32)	
			add_custom_command(
				TARGET ${target}
				POST_BUILD				
				COMMAND ${CMAKE_BINARY_DIR}/gradle/gradlew assembleDebug
				COMMAND ${CMAKE_BINARY_DIR}/gradle/gradlew --stop
				COMMENT "Build APK - ${target}"			
				BYPRODUCTS apk/${target}-debug.apk
			)	
		else()
			add_custom_command(
				TARGET ${target}
				POST_BUILD				
				COMMAND ${CMAKE_BINARY_DIR}/gradle/gradlew assembleDebug
				COMMENT "Build APK - ${target}"			
				BYPRODUCTS apk/${target}-debug.apk
			)		
		endif()

		add_dependencies(${target} gradlew)

	endmacro(add_workspace_executable)

endif()