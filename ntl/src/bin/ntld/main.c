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
#include <glib.h>
#include <gnet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* a network peer that receives log traces and broadcasts them to
 * listeners.
 */

typedef void (*connection_func)(GConn* conn);
typedef void (*data_func)(GConn* conn, const char* data);

typedef struct {
    connection_func new;
    connection_func close;
    data_func       read;
} ConnHandling;

static GServer* logs = NULL;
static GServer* broad = NULL;

static ConnHandling* logger = NULL;
static ConnHandling* listener = NULL;

static GPtrArray* listeners = NULL;

static void send(gpointer d, gpointer ud)
{
    gchar* ln = (gchar*) ud;
    guint  len = strlen(ln) + 1;
    gnet_conn_write((GConn*) d, ln, len);
}

static void broadcast(const gchar* ln)
{
    g_ptr_array_foreach(listeners, send, (gpointer) ln);
}

static void read_log_line(GConn* conn, const char* data)
{
    broadcast(data);
    gnet_conn_readline(conn);
}

static void remove_listener(GConn* conn)
{
    g_ptr_array_remove(listeners, conn);
}

static void activity(GConn* conn, GConnEvent* event, gpointer ud)
{
    ConnHandling* ch = (ConnHandling*) ud;
    switch (event->type) {
        case GNET_CONN_READ:
            if ( ch->read ) {
                (*ch->read)(conn, event->buffer);
            }
            break;
            
        case GNET_CONN_WRITE:
            ; /* Do nothing */
            break;
            
        case GNET_CONN_CLOSE:
            if ( ch->close ) {
                (*ch->close)(conn);
            }            
            break;

        case GNET_CONN_TIMEOUT:
        case GNET_CONN_ERROR:
            gnet_conn_unref(conn);
            break;
            
        default:
            g_assert_not_reached();
    }
}

static void new_logger(GConn* conn)
{
    /* start a read - blocks until data */
    gnet_conn_readline(conn);
}

static void new_listener(GConn* conn)
{
    g_ptr_array_add(listeners, conn);
}

static void on_connection(GServer* serv, GConn* conn, gpointer ud)
{
    if ( conn ) {
        ConnHandling* ch = (ConnHandling*) ud;
        gnet_conn_set_callback(conn, activity, ch);
        gnet_conn_set_watch_error(conn, TRUE);
        (*ch->new)(conn);
    }
}

static void create_servers()
{
    logger = g_new(ConnHandling, 1);
    logger->new = new_logger;
    logger->read = read_log_line;
    logger->close = NULL;

    listener = g_new(ConnHandling, 1);
    listener->new = new_listener;
    listener->read = NULL;
    listener->close = remove_listener;

    logs = gnet_server_new(NULL, 4242, on_connection, logger);
    broad = gnet_server_new(NULL, 4243, on_connection, listener);
}

static void cleanup(void)
{
    gnet_server_delete(broad);
    gnet_server_delete(logs);
    g_free(logger);
    g_free(listener);
    g_ptr_array_free(listeners, TRUE);
}

static void sig_interrupt(int sign)
{
    cleanup();
    exit(EXIT_FAILURE);
}

static void run_main_event_loop()
{
    listeners = g_ptr_array_new();
    GMainLoop* ml = g_main_new(FALSE);

    create_servers();
    signal(SIGINT, sig_interrupt);

    g_main_run(ml);

    cleanup();
}

int main(int argc, char* argv[])
{
    gnet_init();

    run_main_event_loop();

    return EXIT_SUCCESS;
}
