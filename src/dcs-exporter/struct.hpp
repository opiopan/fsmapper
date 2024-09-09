//
// struct.cpp:
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

namespace lua_struct {
    void register_meta_table(lua_State* L);
    int create_struct(lua_State* L);
}
