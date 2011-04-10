include_directories(../src/include/)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GNET_INCLUDE_DIRS})

add_executable(ntld
	main.c)
target_link_libraries(ntld ${GLIB_LIBRARIES} ${GNET_LIBRARIES})
