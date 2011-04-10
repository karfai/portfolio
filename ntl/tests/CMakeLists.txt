include_directories(../src/include/)
include_directories(${GLIB_INCLUDE_DIRS})

add_executable(all_tests
	trace_tests.c trace_tests.h
	decode_tests.c decode_tests.h
	main.c)
target_link_libraries(all_tests ntlc ntll)
target_link_libraries(all_tests ${GLIB_LIBRARIES})
target_link_libraries(all_tests ${GNET_LIBRARIES})
target_link_libraries(all_tests cmockery)
