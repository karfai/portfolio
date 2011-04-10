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
#include "trace_tests.h"

#include "ntlc.h"
#include "cmockery_all.h"
#include <glib.h>

static gchar* actual_sent = NULL;
static time_t mock_time = 5555;
static long mock_millis = 42;

static void mock_send(const char* pkt)
{
    actual_sent = g_strdup_printf("%s", pkt);
}

static void mock_timestamp(time_t* tm, long* millis)
{
    *tm = mock_time;
    *millis = mock_millis;
}

void test_trace(void** state)
{
    gchar prog[] = "test_trace";
    gchar tag[] = "tag";
    gchar mod[] = "module";
    const gchar* fn = __FUNCTION__;
    gchar str[] = "some string";
    int   i = 24;
    gchar fmt[] = "test msg (s=%s, i=%i)";
    gchar *msg = g_strdup_printf(fmt, str, i);
    ntl_TraceLevelT tl = ntl_tl_Debug;

    ntl_setup_override(prog, mock_send, mock_timestamp);
    ntl_trace(tl, tag, mod, fn, fmt, str, i);
    ntl_teardown();
    
    gchar* expected_sent = g_strdup_printf(
        "{ pn:%s, pid:%u, tid:%lu, tl:%u, tm:%lu, millis:%lu, tag:%s, mod:%s, fn:%s, msg:%s }",
        prog, getpid(), ntl_util_gettid(), (guint) tl, mock_time, mock_millis, tag, mod, fn, msg);

    assert_string_equal(expected_sent, actual_sent);

    g_free(msg);
    g_free(actual_sent);
    g_free(expected_sent);
}
