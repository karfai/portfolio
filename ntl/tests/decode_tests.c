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
#include "decode_tests.h"

#include "ntll.h"
#include "cmockery_all.h"
#include <glib.h>

void test_decode(void** state)
{
    gchar prog[] = "test_trace";
    gchar tag[] = "tag";
    gchar mod[] = "module";
    const gchar* fn = __FUNCTION__;
    gchar msg[] = "the msg";
    ntl_TraceLevelT tl = ntl_tl_Debug;
    time_t t = 5555;
    unsigned long millis = 42;
    unsigned int pid = 1122;
    unsigned int tid = 3344;

    gchar* wire_pkt = g_strdup_printf(
        "{ pn:%s, pid:%u, tid:%u, tl:%u, tm:%lu, millis:%lu, tag:%s, mod:%s, fn:%s, msg:%s }",
        prog, pid, tid, (guint) tl, t, millis, tag, mod, fn, msg);

    ntl_Packet* pkt = ntl_packet_decode(wire_pkt);

    assert_false(NULL == pkt);
    assert_string_equal(prog, pkt->prog);
    assert_int_equal(pid, pkt->pid);
    assert_int_equal(tid, pkt->tid);
    assert_true(tl == pkt->lvl);
    assert_int_equal(t, pkt->time);
    assert_int_equal(millis, pkt->millis);
    assert_string_equal(tag, pkt->tag);
    assert_string_equal(mod, pkt->mod);
    assert_string_equal(fn, pkt->fn);
    assert_string_equal(msg, pkt->msg);

    g_free(wire_pkt);
    ntl_packet_free(pkt);
}
