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
#ifndef __ntl_types_h_
#define __ntl_types_h_

#include <time.h>

typedef enum {
    ntl_tl_Trace,
    ntl_tl_Debug,
    ntl_tl_Warn,
    ntl_tl_Error,
} ntl_TraceLevelT;

typedef struct {
    char*           prog;
    unsigned int    pid;
    unsigned int    tid;
    ntl_TraceLevelT lvl;
    time_t          time;
    long            millis;
    char*           tag;
    char*           mod;
    char*           fn;
    char*           msg;
} ntl_Packet;

typedef struct _s_ntl_listener ntl_Listener;

#endif
