//
// luac_mod.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include "mapperplugin_luac.h"
#include <mutex>

namespace luac_mod{
    bool mark_async_source_signaled(FSMAPPER_LUAC_ASYNC_SOURCE source);
    void dispatch_async_events(std::unique_lock<std::mutex>& lock);
    void cleanup_async_sources();
}
