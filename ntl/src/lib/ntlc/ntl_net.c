/*
 *  Part of "NTL" - a simple network logging system
 *
 *  Copyright 2011 Don Kelly <karfai@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "ntl_net.h"

#include <glib.h>
#include <gnet.h>
#include <string.h>

/*
 * A simplifying wrapper around GTcpSocket which posts log traces to
 * the ntl daemon.
 */

struct _s_ntl_net {
    GTcpSocket* sock;
};

ntl_Net* ntl_net_new(void)
{
    ntl_Net* rv = g_new(ntl_Net, 1);
    gnet_init();
    {
        GInetAddr* a = gnet_inetaddr_new("localhost", 4242);
        rv->sock = gnet_tcp_socket_new(a);
        gnet_inetaddr_delete(a);
    }
    
    return rv;
}

void ntl_net_send(ntl_Net* n, const char* pkt)
{
    if ( n && n->sock ) {
        GIOChannel* chan = gnet_tcp_socket_get_io_channel(n->sock);
        gchar* data = g_strdup_printf("%s\n", pkt);
        gsize wrote = 0;
        
        gnet_io_channel_writen(chan, data, strlen(data), &wrote);
        g_free(data);
    }
}

void ntl_net_free(ntl_Net* n)
{
    gnet_tcp_socket_delete(n->sock);
    g_free(n);
}
