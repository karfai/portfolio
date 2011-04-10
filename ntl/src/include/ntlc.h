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
#ifndef __ntlc_h_
#define __ntlc_h_
/*
 * The public interface used by a logging client to post traces.
 */

#include <time.h>

typedef enum {
    ntl_tl_Trace,
    ntl_tl_Debug,
    ntl_tl_Warn,
    ntl_tl_Error,
} ntl_TraceLevelT;

typedef void (*ntl_send_func)(const char* pkt);
typedef void (*ntl_timestamp_func)(time_t* t, long* millis);

void ntl_setup(const char* program_name);
void ntl_setup_override(const char* program_name, ntl_send_func send_func, ntl_timestamp_func ts_func);
void ntl_teardown(void);
void ntl_trace(ntl_TraceLevelT tl, const char* tag, const char* mod, const char* fn, const char* fmt, ...) __attribute__ ((format (printf, 5, 6)));

unsigned long ntl_util_gettid(void);

#endif
