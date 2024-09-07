//
// utils.cpp: Utility functions for DCS exporter Lua script
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//
//  Note: 
//    This file contains code for a C-module that can be loaded into the DCS World Lua environment.
//    Therefore, the Lua version targeted by this file is 5.1.
//

#include <windows.h>

#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <format>
#include <optional>
#include <algorithm>

#include <stdlib.h>
#undef min
#undef max

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

//============================================================================================
// Utilities
//============================================================================================
inline std::string numeral(int num){
    return num == 1 ? "1st" : 
           num == 2 ? "2nd" :
           num == 3 ? "3rd" : 
           std::format("{}th", num);
}

inline lua_Integer check_lua_arg_integer(lua_State* L, int index){
    if (lua_type(L, index) != LUA_TNUMBER){
        throw std::runtime_error(std::format("the {} argument type must be number", numeral(index)));
    }
    return lua_tointeger(L, index);
}

inline lua_Number check_lua_arg_number(lua_State* L, int index){
    if (lua_type(L, index) != LUA_TNUMBER){
        throw std::runtime_error(std::format("the {} argument type must be number", numeral(index)));
    }
    return lua_tonumber(L, index);
}

inline const char* check_lua_arg_lstring(lua_State* L, int index, size_t* length){
    if (lua_type(L, index) != LUA_TSTRING){
        throw std::runtime_error(std::format("the {} argument type must be string", numeral(index)));
    }
    return lua_tolstring(L, index, length);
}

//============================================================================================
// Binary translator
//============================================================================================
class safe_buffer{
    std::vector<char> buf;
public:
    static constexpr auto initial_length{256};
    static constexpr auto expand_unit{512};
    safe_buffer(size_t length = initial_length){
        buf.resize(length);
    }
    void ensure_length(size_t length){
        if (buf.size() < length){
            auto new_length = (length / expand_unit + 1) * expand_unit;
            buf.resize(new_length);
        }
    }
    size_t length()const{return buf.size();}
    operator char* (){return &buf[0];}
    operator const char *()const{return &buf[0];}
};

enum class translation_type{nil, integer, number, string};

struct translation_unit{
    translation_type type{translation_type::nil};
    bool is_fixed_length{true};
    size_t min_length{0};
    int alignment{1};
    bool is_associated{false};
    int pack_index{0};
    bool is_pushing_value{true};
    std::optional<int> bias;

    inline size_t padding(size_t pos)const{
        auto modulo = pos % alignment;
        return modulo ? alignment - modulo : 0;
    }

    virtual size_t pack(lua_State* L, safe_buffer& buf, size_t offset) = 0;
    virtual size_t unpack(lua_State* L, const char* in, size_t in_length, size_t offset) = 0;
};
using unit_list = std::vector<std::unique_ptr<translation_unit>>;

struct integer_unit : public translation_unit{
    int precision;
    bool is_signed;
    void (*put_data)(char* to, lua_Integer value, int precision);
    lua_Integer (*get_data)(const char* from, int precision);

    integer_unit(int precision, bool is_signed) : precision(precision), is_signed(is_signed){
        type = translation_type::integer;
        min_length = precision;
        alignment = (precision == 2 || precision == 4 || precision == 8) ? precision : 1;
        if (precision == 1 && is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<int8_t*>(to) = static_cast<int8_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const int8_t*>(from));};
        }else if (precision == 1 && !is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<uint8_t*>(to) =  static_cast<uint8_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const uint8_t*>(from));};
        }else if (precision == 2 && is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<int16_t*>(to) = static_cast<int16_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const int16_t*>(from));};
        }else if (precision == 2 && !is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<uint16_t*>(to) = static_cast<uint16_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const uint16_t*>(from));};
        }else if (precision == 4 && is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<int32_t*>(to) = static_cast<int32_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const int32_t*>(from));};
        }else if (precision == 4 && !is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<uint32_t*>(to) = static_cast<uint32_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const uint32_t*>(from));};
        }else if (precision == 8 && is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<int64_t*>(to) = static_cast<int64_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const int64_t*>(from));};
        }else if (precision == 8 && !is_signed){
            put_data = [](auto to, auto value, auto){*reinterpret_cast<uint64_t*>(to) = static_cast<uint64_t>(value);};
            get_data = [](auto from, auto){return static_cast<lua_Integer>(*reinterpret_cast<const uint64_t*>(from));};
        }else if (precision < 8 && precision > 0){
            put_data = [](auto to, auto value, auto precision){
                for (auto i = 0; i < precision; i++){
                    to[i] = value & 0xff;
                    value = value >> 8;
                }
            };
            if (is_signed){
                get_data = [](auto from, auto precision) {
                    lua_Integer value{0};
                    auto value_p{reinterpret_cast<char*>(&value)};
                    auto i{0};
                    for (; i < precision; i++){
                        value_p[i] = from[i];
                    }
                    if (value_p[i - 1] & 0x80){
                        for (; i < 8; i++){
                            value_p[i] = -1;
                        }
                    }
                    return value;
                };
            }else{
                get_data = [](auto from, auto precision) {
                    lua_Integer value{0};
                    auto value_p{reinterpret_cast<char*>(&value)};
                    auto i{0};
                    for (; i < precision; i++){
                        value_p[i] = from[i];
                    }
                    return value;
                };
            }
        }else{
            throw std::runtime_error(std::format("specified integer value width [{}] is invalid", precision));
        }
    }

    size_t pack(lua_State *L, safe_buffer &buf, size_t offset) override{
        auto value = check_lua_arg_integer(L, pack_index);
        offset += padding(offset);
        auto goal = offset + precision;
        buf.ensure_length(goal);
        put_data(&buf[offset], value + (bias ? *bias : 0), precision);
        return goal;
    }

    size_t unpack(lua_State *L, const char *in, size_t in_length, size_t offset) override{
        offset += padding(offset);
        auto goal = offset + precision;
        if (goal <= in_length){
            auto value = get_data(in + offset, precision);
            value -= bias ? *bias : 0;
            lua_pushinteger(L, value);
        }
        return goal;
    };
};

template <typename T>
struct float_unit : public translation_unit{
    float_unit(){
        type = translation_type::number;
        min_length = sizeof(T);
        alignment = sizeof(T);
    }

    size_t pack(lua_State *L, safe_buffer &buf, size_t offset) override{
        auto value = check_lua_arg_number(L, pack_index);
        offset += padding(offset);
        auto goal = offset + min_length;
        buf.ensure_length(goal);
        *reinterpret_cast<T*>(&buf[offset]) = static_cast<T>(value);
        return goal;
    }

    size_t unpack(lua_State *L, const char *in, size_t in_length, size_t offset) override{
        offset += padding(offset);
        auto goal = offset + min_length;
        if (goal <= in_length){
            auto value = *reinterpret_cast<const T*>(in + offset);
            lua_pushnumber(L, value);
        }
        return goal;
    }
};

struct fixed_string_unit : public translation_unit{

    fixed_string_unit(size_t length) {
        if (length < 1){
            throw std::runtime_error(std::format("specified fixed string length [{}] is invalid", length));
        }
        type = translation_type::string;
        min_length = length;
    }

    size_t pack(lua_State *L, safe_buffer &buf, size_t offset) override{
        size_t input_len;
        auto value = check_lua_arg_lstring(L, pack_index, &input_len);
        auto goal = offset + min_length;
        buf.ensure_length(goal);
        auto copy_len = std::min(input_len, min_length);
        memcpy(&buf[offset], value, copy_len);
        if (min_length > copy_len){
            memset(&buf[offset + copy_len], 0, min_length - copy_len);
        }
        return goal;
    }

    size_t unpack(lua_State *L, const char *in, size_t in_length, size_t offset) override{
        auto goal = offset + min_length;
        if (goal <= in_length){
            lua_pushlstring(L, in + offset, min_length);
        }
        return goal;
    }
};

struct associated_string_length_unit : public integer_unit{
    lua_Integer parsed_value{0};

    associated_string_length_unit(int precision) : integer_unit(precision, false){
        type = translation_type::integer;
        is_associated = true;
        is_pushing_value = false;
    }

    auto get_parsed_value()const{return parsed_value;}

    size_t pack(lua_State *L, safe_buffer &buf, size_t offset) override{
        size_t length;
        auto value = check_lua_arg_lstring(L, pack_index, &length) + (bias ? *bias : 0);
        offset += padding(offset);
        auto goal = offset + precision;
        buf.ensure_length(goal);
        put_data(&buf[offset], length, precision);
        return goal;
    }

    size_t unpack(lua_State *L, const char *in, size_t in_length, size_t offset) override{
        offset += padding(offset);
        auto goal = offset + precision;
        if (goal <= in_length){
            parsed_value = get_data(in + offset, precision);
            parsed_value -= bias ? *bias : 0;
        }
        return goal;
    };
};

struct variable_string_unit : public translation_unit{
    const associated_string_length_unit& length_unit;

    variable_string_unit(const associated_string_length_unit& length_unit) : length_unit(length_unit){
        type = translation_type::string;
        is_fixed_length = false;
    }

    size_t pack(lua_State *L, safe_buffer &buf, size_t offset) override{
        size_t input_len;
        auto value = check_lua_arg_lstring(L, pack_index, &input_len);
        auto goal = offset + input_len;
        buf.ensure_length(goal);
        memcpy(&buf[offset], value, input_len);
        return goal;
    }

    size_t unpack(lua_State *L, const char *in, size_t in_length, size_t offset) override{
        auto goal = offset + length_unit.get_parsed_value();
        if (goal <= in_length){
            lua_pushlstring(L, in + offset, length_unit.get_parsed_value());
        }
        return goal;
    }
};

static inline int next_pack_index(const unit_list& rules) {
    return rules.size() ? rules.back()->pack_index + 1 : 2; // note: 1st argument is self
};

class bin_translator{
    std::string format;
    unit_list rules;
    size_t packsize{0};
    safe_buffer buf;

public:
    bin_translator() = delete;
    bin_translator(const char* format) : format(format){
        struct convert_option{
            char name;
            bool has_attribute;
            bool attribute_is_optional;
            int default_attribute;
            void (*add_rule)(unit_list& rules, int attribute);
        };
        static std::unordered_map<char, convert_option> options{
            {'f', {'f', false, false, 0, [](unit_list& rules, auto attribute){
                auto rule = std::make_unique<float_unit<float>>();
                rule->pack_index = next_pack_index(rules);
                rules.emplace_back(std::move(rule));
            }}},
            {'d', {'d', false, false, 0, [](unit_list& rules, auto attribute){
                auto rule = std::make_unique<float_unit<double>>();
                rule->pack_index = next_pack_index(rules);
                rules.emplace_back(std::move(rule));
            }}},
            {'i', {'i', true, true, 8, [](unit_list& rules, auto attribute){
                auto rule = std::make_unique<integer_unit>(attribute, true);
                rule->pack_index = next_pack_index(rules);
                rules.emplace_back(std::move(rule));
            }}},
            {'I', {'I', true, true, 8, [](unit_list& rules, auto attribute){
                auto rule = std::make_unique<integer_unit>(attribute, false);
                rule->pack_index = next_pack_index(rules);
                rules.emplace_back(std::move(rule));
            }}},
            {'c', {'c', true, false, 0, [](unit_list& rules, auto attribute){
                auto rule = std::make_unique<fixed_string_unit>(attribute);
                rule->pack_index = next_pack_index(rules);
                rules.emplace_back(std::move(rule));
            }}},
            {'s', {'s', true, true, 8, [](unit_list& rules, auto attribute){
                auto rule_len = std::make_unique<associated_string_length_unit>(attribute);
                auto rule_body = std::make_unique<variable_string_unit>(*rule_len);
                rule_len->pack_index = next_pack_index(rules);
                rule_body->pack_index = rule_len->pack_index;
                rules.emplace_back(std::move(rule_len));
                rules.emplace_back(std::move(rule_body));
            }}},
            {'S', {'S', true, false, 0, [](unit_list& rules, auto attribute){
                if (attribute < 1 && attribute > rules.size()){
                    throw std::runtime_error(std::format("specified length field index [{}] for the string is invalid", attribute));
                }else if (rules[attribute - 1]->type != translation_type::integer){
                    throw std::runtime_error(std::format("specified length field index [{}] for string refer other than integer field", attribute));
                }
                for (auto i = attribute - 1; i < rules.size(); i++){
                    rules[i]->pack_index--;
                }
                auto rule_len = std::make_unique<associated_string_length_unit>(static_cast<int>(rules[attribute - 1]->min_length));
                auto rule_body = std::make_unique<variable_string_unit>(*rule_len);
                rule_len->pack_index = next_pack_index(rules);
                rule_body->pack_index = rule_len->pack_index;
                rules[attribute - 1] =  std::move(rule_len);
                rules.emplace_back(std::move(rule_body));
            }}},
        };

        const convert_option* current_option{nullptr};
        std::optional<int> attribute{std::nullopt};
        std::optional<int> bias{std::nullopt};
        auto add_rule = [&]{
            if (current_option){
                if (!current_option->has_attribute && attribute){
                    throw std::runtime_error(std::format("numeric attribute cannot be specified for the translation option [{}]", current_option->name));
                }
                if (current_option->has_attribute && !current_option->attribute_is_optional && !attribute){
                    throw std::runtime_error(std::format("numeric attribute must be specified for the translation option [{}]", current_option->name));
                }
                current_option->add_rule(rules, attribute ? *attribute : current_option->default_attribute);
                rules.back()->bias = bias;
            }
            current_option = nullptr;
            attribute = std::nullopt;
            bias = std::nullopt;
        };
        enum class parse_state{option, attribute, bias} state{parse_state::option};
        for (; *format; format++){
            if (state == parse_state::option){
                add_rule();
                if (*format == ' '){
                    continue;
                }else if (options.count(*format) == 0){
                    throw std::runtime_error(std::format("invalid translation option [{}]", *format));
                }
                current_option = &options[*format];
                if (format[1] >= '0' && format[1] <= '9') {
                    state = parse_state::attribute;
                }
            }else if (state == parse_state::attribute){
                if (*format >= '0' && *format <= '9'){
                    attribute = (attribute ? *attribute * 10 : 0) + (*format - '0');
                }else if (*format == ':'){
                    state = parse_state::bias;
                }else{
                    state = parse_state::option;
                    format--;
                }
            }else{ // parse_state::bias
                if (*format >= '0' && *format <= '9'){
                    bias = (bias ? *bias * 10 : 0) + (*format - '0');
                }else{
                    state = parse_state::option;
                    format--;
                }
            }
        }
        add_rule();

        for (auto& rule : rules){
            packsize += rule->padding(packsize) + rule->min_length;
        }
    }

    operator const char* (){return buf;}

    size_t get_packsize() const{return packsize;}

    size_t pack(lua_State* L){
        size_t length{0};
        for (auto& rule : rules){
            length = rule->pack(L, buf, length);
        }
        return length;
    }

    int unpack(lua_State* L, const char* in, size_t in_length, size_t offset){
        int pushed_num{0};
        for (auto& rule : rules){
            offset = rule->unpack(L, in, in_length, offset);
            if (offset > in_length){
                break;
            }
            if (rule->is_pushing_value){
                pushed_num++;
            }
        }
        return pushed_num;
    }
};

//============================================================================================
// Lua C functions
//============================================================================================
static const char* l_struct_type_name = "fsmapper_struct";

static int l_struct(lua_State* L){
    auto format = luaL_checkstring(L, 1);
    auto object = lua_newuserdata(L, sizeof(bin_translator));
    auto user_data = lua_gettop(L);
    luaL_getmetatable(L, l_struct_type_name);
    lua_setmetatable(L, user_data);

    try{
        new(object) bin_translator{format};
    }catch (std::runtime_error& e){
        return luaL_argerror(L, 1, e.what());
    }

    return 1;
}

static int l_struct_gc(lua_State* L){
    auto data = lua_touserdata(L, 1);
    if (data){
        reinterpret_cast<bin_translator*>(data)->~bin_translator();
    }
    return 0;
}

static int l_struct_packsize(lua_State* L){
    auto udata = luaL_checkudata(L, 1, l_struct_type_name);
    try{
        auto packsize = reinterpret_cast<bin_translator*>(udata)->get_packsize();
        lua_pushinteger(L, packsize);
        return 1;
    }catch (std::runtime_error& e){
        return luaL_error(L, "%s", e.what());
    }
}

static int l_struct_pack(lua_State* L){
    auto udata = luaL_checkudata(L, 1, l_struct_type_name);
    try{
        auto translator = reinterpret_cast<bin_translator*>(udata);
        auto length = translator->pack(L);
        lua_pushlstring(L, *translator, length);
        return 1;
    }catch (std::runtime_error& e){
        return luaL_error(L, "%s", e.what());
    }
}

static int l_struct_unpack(lua_State* L){
    auto udata = luaL_checkudata(L, 1, l_struct_type_name);
    size_t in_length;
    auto in = luaL_checklstring(L, 2, &in_length);
    size_t offset{0};
    if (lua_gettop(L) > 2){
        offset = luaL_checkinteger(L, 3);
        if (offset < 0 || offset > in_length){
            return luaL_error(L, "the 3rd argument value is out of range");
        }
    }
    try{
        return reinterpret_cast<bin_translator*>(udata)->unpack(L, in, in_length, offset);
    }catch (std::runtime_error& e){
        return luaL_error(L, "%s", e.what());
    }
}

//============================================================================================
// Lua C module entry point
//============================================================================================
static luaL_Reg module[]{
    {"struct", l_struct},
    {nullptr, nullptr},
};

extern "C" __declspec(dllexport) int luaopen_fsmapper_utils(lua_State* L){
    // register a metatable for "struct"
    luaL_newmetatable(L, l_struct_type_name);
    auto meta_table = lua_gettop(L);
    lua_pushcfunction(L, l_struct_gc);
    lua_setfield(L, meta_table, "__gc");
    lua_newtable(L);
    auto index_table = lua_gettop(L);
    lua_pushcfunction(L, l_struct_packsize);
    lua_setfield(L, index_table, "packsize");
    lua_pushcfunction(L, l_struct_pack);
    lua_setfield(L, index_table, "pack");
    lua_pushcfunction(L, l_struct_unpack);
    lua_setfield(L, index_table, "unpack");
    lua_setfield(L, meta_table, "__index");
    lua_pop(L, 1);

    // register module
    luaL_register(L, "fsmapper_utils", module);
    return 1;
}

//============================================================================================
// DLL entry point
//============================================================================================
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call){
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
