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
#include "ntlc.h"

#include "ntl_net.h"
#include <glib.h>
#include <glib/gprintf.h>
#include <gnet.h>

/*
 * The implementation of the public interface described in
 * include/ntlc.h.
 */
typedef struct _s_ntl_block {
    ntl_send_func      send;
    ntl_timestamp_func timestamp;
    ntl_Net*           net;
} ntl_Block;

static void internal_send(const char* pkt);
static void internal_timestamp(time_t* tm, long* millis);

static ntl_Block block = {
    .send = internal_send,
    .timestamp = internal_timestamp,
    .net = NULL,
};

static void internal_send(const char* pkt)
{
    ntl_net_send(block.net, pkt);
}

static void internal_timestamp(time_t* tm, long* millis)
{
    GTimeVal tv;
    g_get_current_time(&tv);
    *tm = tv.tv_sec;
    *millis = tv.tv_usec / 1000;
}

static void internal_setup(const char* program_name, ntl_send_func send_func, ntl_timestamp_func ts_func)
{
    g_set_prgname(program_name);
    if ( send_func ) {
        block.send = send_func;
    }
    if ( ts_func ) {
        block.timestamp = ts_func;
    }
    block.net = ntl_net_new();
}

void ntl_setup_override(const char* program_name, ntl_send_func send_func, ntl_timestamp_func ts_func)
{
    internal_setup(program_name, send_func, ts_func);
}

void ntl_setup(const char* program_name)
{
    internal_setup(program_name, NULL, NULL);
}

void ntl_teardown(void)
{
    ntl_net_free(block.net);
}

void ntl_trace(ntl_TraceLevelT tl, const char* tag, const char* mod, const char* fn, const char *fmt, ...)
{
    gchar* msg = NULL;
    {
        va_list args;
        va_start(args, fmt);
        msg = g_strdup_vprintf(fmt, args);
        va_end(args);
    }

    time_t st = 0;
    long millis = 0;
    (*block.timestamp)(&st, &millis);
    gchar* pkt = g_strdup_printf(
        "{ pn:%s, pid:%u, tid:%lu, tl:%u, tm:%lu, millis:%lu, tag:%s, mod:%s, fn:%s, msg:%s }",
        g_get_prgname(), getpid(), ntl_util_gettid(), (guint) tl, st, millis, tag, mod, fn, msg);
    g_free(msg);
    (*block.send)(pkt);
    g_free(pkt);
}

