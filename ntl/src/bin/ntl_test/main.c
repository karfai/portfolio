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
#include <stdio.h>
#include "ntlc.h"

/* a test program to post log traces */

int main(int argc, char* argv[]) {
    ntl_setup("ntl_test");
    ntl_trace(ntl_tl_Trace, "tag0", "test", __FUNCTION__, "A string (%s) and a number (%i)", "else", 42);
    ntl_trace(ntl_tl_Warn, "tag1", "test", __FUNCTION__, "A warning");
    ntl_trace(ntl_tl_Error, "tag1", "test", __FUNCTION__, "An error");
    ntl_trace(ntl_tl_Debug, "tag0", "test", __FUNCTION__, "Debug level logging");
    ntl_trace(ntl_tl_Debug, "tag0", "test", __FUNCTION__, "More debug logging");
    ntl_trace(ntl_tl_Trace, "tag1", "test", __FUNCTION__, "Another trace log");
    ntl_teardown();
    return 0;
}
