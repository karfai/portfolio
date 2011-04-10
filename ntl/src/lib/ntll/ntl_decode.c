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

/*
 * Procedures for decoding the trace strings sent across the network.
 */

/* private */
static gchar pat[] = "(\\w+)\\:([^\\,|^\\}]+)";
static GRegex* re = NULL;

void free_string(gpointer d)
{
    g_free((gchar*) d);
}

static GHashTable* parse(const char* pkt)
{
    GHashTable* rv = g_hash_table_new_full(g_str_hash, g_str_equal, free_string, free_string);
    if ( NULL == re ) {
        re = g_regex_new(pat, G_REGEX_OPTIMIZE, 0, NULL);
    }
    
    {
        GMatchInfo* mi = NULL;
        g_regex_match(re, pkt, 0, &mi);
        while ( g_match_info_matches(mi) ) {
            gchar* k = g_match_info_fetch(mi, 1);
            gchar* v = g_match_info_fetch(mi, 2);
            g_hash_table_replace(rv, k, v);
            g_match_info_next(mi, NULL);
        }
        g_match_info_free(mi);
    }

    return rv;
}

/* public */
ntl_Packet* ntl_packet_decode(const char* pkt)
{
    ntl_Packet* rv = g_new(ntl_Packet, 1);
    GHashTable* ht = parse(pkt);
    rv->prog = g_strdup(g_hash_table_lookup(ht, "pn"));
    rv->pid = atol(g_hash_table_lookup(ht, "pid"));
    rv->tid = atol(g_hash_table_lookup(ht, "tid"));
    rv->lvl = (ntl_TraceLevelT) atoi(g_hash_table_lookup(ht, "tl"));
    rv->time = atoi(g_hash_table_lookup(ht, "tm"));
    rv->millis = atol(g_hash_table_lookup(ht, "millis"));
    rv->tag = g_strdup(g_hash_table_lookup(ht, "tag"));
    rv->mod = g_strdup(g_hash_table_lookup(ht, "mod"));
    rv->fn = g_strdup(g_hash_table_lookup(ht, "fn"));
    rv->msg = g_strchomp(g_strdup(g_hash_table_lookup(ht, "msg")));
    g_hash_table_destroy(ht);
    return rv;
}

void ntl_packet_free(ntl_Packet* pkt)
{
    g_free(pkt->prog);
    g_free(pkt->tag);
    g_free(pkt->mod);
    g_free(pkt->fn);
    g_free(pkt->msg);
    g_free(pkt);
}

