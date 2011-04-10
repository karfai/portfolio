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
#ifndef __ntll_h_
#define __ntll_h_
/*
 * The public interface used to construct a listener: a client which
 * receives log traces from the log daemon.
 */

#include "ntl_types.h"
#include <glib.h>

ntl_Packet* ntl_packet_decode(const char* pkt);
void        ntl_packet_free(ntl_Packet* pkt);

const char* ntl_level_to_string(ntl_TraceLevelT lvl);

typedef void (*ntl_listener_pkt_func)(const ntl_Packet* pkt, gpointer data);

ntl_Listener* ntl_listener_new(const char* host, ntl_listener_pkt_func pkt_func, gpointer data);
gchar*        ntl_listener_default_time_format(const ntl_Packet* pkt);
void          ntl_listener_free(ntl_Listener* l);

#endif
