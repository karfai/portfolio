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

/*
 * Utility functions for formatting NTL types.
 */

const char* ntl_level_to_string(ntl_TraceLevelT lvl)
{
    switch (lvl) {
        case ntl_tl_Trace:
            return "ntl_tl_Trace";

        case ntl_tl_Debug:
            return "ntl_tl_Debug";

        case ntl_tl_Warn:
            return "ntl_tl_Warn";

        case ntl_tl_Error:
            return "ntl_tl_Error";
        
        default:
            ;
    }

    return "<unknown>";
}
