/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "systemwrappers.h"
#include "logging.h"
#include "GlobalState.h"
#include "../shared/SharedConfig.h"
#include "backtrace.h"
#include <execinfo.h>

namespace libtas {

DEFINE_ORIG_POINTER(getpid);

/* Override */ pid_t getpid (void) throw()
{
    LINK_NAMESPACE_GLOBAL(getpid);
    pid_t pid = orig::getpid();
    
    if (GlobalState::isNative()) {
        return pid;
    }

    DEBUGLOGCALL(LCF_SYSTEM);

    /* A bit hackish: Dead Cells seeds the rng using the process pid.
     * However, specifying a wrong pid for other calls will most likely crash
     * games (mainly because OpenGL drivers need this value). So we look from
     * which library was the call made. */
    void* return_address =  __builtin_return_address(0);
    char** symbols = backtrace_symbols(&return_address, 1);
    if (symbols != nullptr) {
        if (strstr(symbols[0], "libhl.so"))
            pid = 1234;
        free(symbols);
    }

    return pid;
}

}
