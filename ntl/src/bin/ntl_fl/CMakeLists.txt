include_directories(../../include/)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GNET_INCLUDE_DIRS})

add_executable(ntl_fl main.c)
target_link_libraries(ntl_fl ntll)
target_link_libraries(ntl_fl ${GLIB_LIBRARIES} ${GNET_LIBRARIES})
