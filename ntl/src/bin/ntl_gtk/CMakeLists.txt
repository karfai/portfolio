include_directories(../../include/)
include_directories(${GTK_INCLUDE_DIRS})
include_directories(${GDK_INCLUDE_DIRS})
include_directories(${GNET_INCLUDE_DIRS})

add_executable(ntl_gtk main.c)
target_link_libraries(ntl_gtk ntll)
target_link_libraries(ntl_gtk ${GTK_LIBRARIES} ${GNET_LIBRARIES})
