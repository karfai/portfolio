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
#include "ntll.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

/* a listener that writes to stdout or to a file */

static ntl_Listener* ltner = NULL;
static GIOChannel*   chan = NULL;

static void write_log(const ntl_Packet* pkt, gpointer d)
{
    gsize       wrote = 0;
    gchar*      ln = NULL;
    gchar*      tm = ntl_listener_default_time_format(pkt);

    ln = g_strdup_printf("[%s] [%s] [%s, %u, %u] [%s] [%s/%s]: %s\n",
        pkt->tag, ntl_level_to_string(pkt->lvl),
        pkt->prog, pkt->pid, pkt->tid,
        tm, pkt->mod, pkt->fn, pkt->msg);
    g_io_channel_write_chars(chan, ln, strlen(ln), &wrote, NULL);
    g_io_channel_flush(chan, NULL);
    g_free(tm);
    g_free(ln);
}

void open_channel(const gchar* fn)
{
    if ( fn ) {
        chan = g_io_channel_new_file(fn, "a", NULL);
    } else {
        chan = g_io_channel_unix_new(fileno(stdout));
    }
}

void connect_listener(void)
{
    ltner = ntl_listener_new("localhost", write_log, NULL);
}

static void cleanup(void)
{
    ntl_listener_free(ltner);
}

static void sig_interrupt(int sign)
{
    cleanup();
    exit(EXIT_FAILURE);
}

static void run_main_event_loop()
{
    GMainLoop* ml = g_main_new(FALSE);

    signal(SIGINT, sig_interrupt);
    g_main_run(ml);
    cleanup();
}

int main(int argc, char* argv[])
{
    gchar* fn = NULL;
    
    if ( argc > 1 ) {
        fn = argv[1];
    }

    gnet_init();
    open_channel(fn);
    connect_listener();
    run_main_event_loop();

    return EXIT_SUCCESS;
}

