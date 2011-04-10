include_directories(../../include)
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${GNET_INCLUDE_DIRS})

add_library(ntlc ntlc.c ntl_util.c ntl_net.c)
