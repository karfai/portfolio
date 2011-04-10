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
#include "ntll.h"

#include <glib.h>
#include <gnet.h>

/*
 * The implementation of a listener.
 */

/* private */
struct _s_ntl_listener {
    GConn*                conn;
    ntl_listener_pkt_func pkt_func;
    gpointer              data;
};

static void activity(GConn* conn, GConnEvent* event, gpointer ud)
{
    ntl_Listener* l = (ntl_Listener*) ud;
    switch (event->type)
    {
        case GNET_CONN_CONNECT:
        {
            gnet_conn_timeout(conn, 0);	/* reset timeout */
            gnet_conn_readline(conn);
        }
        break;
        
        case GNET_CONN_READ:
        {
            ntl_Packet* pkt = ntl_packet_decode(event->buffer);
            (*l->pkt_func)(pkt, l->data);
            ntl_packet_free(pkt);
            gnet_conn_readline(conn);
            
            break;
        }
        
        case GNET_CONN_CLOSE:
        case GNET_CONN_TIMEOUT:
        case GNET_CONN_ERROR:
            break;

    default:
      g_assert_not_reached ();
    }
}

/* public */
ntl_Listener* ntl_listener_new(const char* host, ntl_listener_pkt_func pkt_func, gpointer data)
{
    ntl_Listener* rv = g_new(ntl_Listener, 1);
    rv->pkt_func = pkt_func;
    rv->data = data;
    rv->conn = gnet_conn_new(host, 4243, activity, rv);
    gnet_conn_set_watch_error(rv->conn, TRUE);
    gnet_conn_timeout(rv->conn, 30000);
    gnet_conn_connect(rv->conn);
    return rv;
}

void ntl_listener_free(ntl_Listener* l)
{
    if ( l ) {
        gnet_conn_disconnect(l->conn);
        gnet_conn_unref(l->conn);
        g_free(l);
    }
}

gchar* ntl_listener_default_time_format(const ntl_Packet* pkt)
{
    struct tm*  tm = localtime(&(pkt->time));
    char        dt_buf[64];

    strftime(dt_buf, 64, "%Y-%m-%d %H:%M:%S", tm);
    return g_strdup_printf("%s.%03lu", dt_buf, pkt->millis);
}
