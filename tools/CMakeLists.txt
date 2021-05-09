cmake_minimum_required( VERSION 3.1.0 )

add_subdirectory( re2c )
add_subdirectory( lemon )
add_subdirectory( zipdir )

if( NSWITCH )
	add_subdirectory( nxlauncher )
endif()

set( CROSS_EXPORTS ${CROSS_EXPORTS} PARENT_SCOPE )
