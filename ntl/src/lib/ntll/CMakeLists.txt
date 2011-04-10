include_directories(../../include)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GNET_INCLUDE_DIRS})

add_library(ntll ntl_decode.c ntl_format.c ntl_listener.c)
