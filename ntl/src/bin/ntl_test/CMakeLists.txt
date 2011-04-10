include_directories(../../include/)
include_directories(${GLIB_INCLUDE_DIRS})

add_executable(ntl_test main.c)
target_link_libraries(ntl_test ntlc)
target_link_libraries(ntl_test ${GLIB_LIBRARIES})
target_link_libraries(ntl_test ${GNET_LIBRARIES})
