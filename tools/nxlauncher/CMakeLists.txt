cmake_minimum_required( VERSION 3.1.0 )

if( NSWITCH )
	set(APP_NAME "GZDoom Launcher")
	set(APP_AUTHOR "fgsfds")
	set(APP_VERSION "nx4.6pre-7")
	set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/gzdoom_launcher.jpg")
	set(APP_RESDIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
	file( GLOB SRC_HDR src/*.h )
	file( GLOB SRC_FILES src/*.c )
	add_executable( gzdoom_launcher ${SRC_HDR} ${SRC_FILES} )
	target_link_libraries( gzdoom_launcher SDL2 EGL glapi drm_nouveau nx m stdc++ )
	add_custom_command( TARGET gzdoom_launcher POST_BUILD
		COMMAND nacptool --create "${APP_NAME}" "${APP_AUTHOR}" "${APP_VERSION}" "gzdoom_launcher.nacp"
		COMMAND aarch64-none-elf-strip -o gzdoom_launcher_stripped gzdoom_launcher
		COMMAND elf2nro gzdoom_launcher_stripped "${CMAKE_BINARY_DIR}/gzdoom_launcher.nro" --icon="${APP_ICON}" --nacp=gzdoom_launcher.nacp --romfsdir="${APP_RESDIR}" )
endif()
