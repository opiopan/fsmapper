//
// lua51checker.hpp:
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

using LUA51CHECKER = void*;
extern "C" __declspec(dllexport) LUA51CHECKER lua51checker_open();
extern "C" __declspec(dllexport) void lua51checker_close(LUA51CHECKER checker);
extern "C" __declspec(dllexport) bool lua51checker_check_syntax(LUA51CHECKER checker, const char* chunk);
extern "C" __declspec(dllexport) const char* lua51checker_get_last_error(LUA51CHECKER checker);

namespace lua51{
    class checker{
        LUA51CHECKER handle;
    public:
        checker(){handle = lua51checker_open();}
        ~checker(){lua51checker_close(handle);}
        bool check_syntax(const char* chunk){return lua51checker_check_syntax(handle, chunk);}
        const char* last_error(){return lua51checker_get_last_error(handle);}
    };
};
