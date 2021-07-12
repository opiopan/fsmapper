#include <iostream>
#include "mappercore.h"

#include <functional>
#include <sol/sol.hpp>

class Test{
    int counter;

public:
    Test():counter(0){};
    Test(int c): counter(c){};
    ~Test(){
        std::cout << "destoroy Test object" << std::endl;
    };

    int add(sol::object val){
        if (val.is<int>()){
            counter += val.as<int>();
            std::cout << "Test::add(): add " << val.as<int>() << std::endl;
        }else{
            std::cout << "Test::add(): invalid argument" << std::endl;
        }
        return counter;
    }

    int value(){
        return counter;
    }
};

std::string concat_string(std::string str1, std::string str2){
    return str1 + str2;
}

class Test2{
public: 
    int foo(){return 10;}
};

static bool test(){
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    auto state = lua.lua_state();

    auto result = lua.safe_script(
        "t = {10.1,20,30,foo=40}\n"
        "function showkv(k, v) print(k, v) end",
        sol::script_pass_on_error);
    if (!result.valid()){
        sol::error err = result;
        std::cerr << "The code has failed to run!\n"
                  << err.what()
                  << std::endl;
        return false;
    }

    sol::object t = lua["t"];
    sol::table table = t;
    sol::object showkv_o = lua["showkv"];
    sol::protected_function showkv = showkv_o;
    for (const auto& kv : table){
        sol::object key = kv.first;
        auto keytype = key.get_type();
        if (keytype == sol::type::table){
            std::cout << "table" << std::endl;
        }else if (keytype == sol::type::string){
            std::cout << "[String Key]: " << key.as<const char*>() <<std::endl;
        }
        sol::object value = kv.second;
        if (value.is<long>()){
            auto numvalue = value.as<long>();
            std::cout << "num value: " << numvalue << std::endl;
        }
        auto result = showkv(key, value);
        if (!result.valid()){
            sol::error err = result;
        }
    }
    sol::object tmp = table["bar"];
    auto type = tmp.get_type();
    if (type == sol::type::lua_nil){
        std::cout << "bar is none" << std::endl;
    }

    auto mapper = lua.create_table();
    lua["mapper"] = mapper;
    auto test_pointer = std::make_shared<Test>();
    lua["test"] = test_pointer;
    sol::object test = lua["test"];
    mapper.new_usertype<Test>(
        "Test",
        "add", &Test::add,
        "value", &Test::value);
    lua.new_usertype<Test2>(
        "Test2",
        "foo", &Test2::foo);
    std::cout << "# check mapper.Test" << std::endl;
    result = lua.safe_script(
        "print(mapper) "
        "print(mapper.Test) "
        "test:add(10) "
        "test:add('hoge') "
        "test:add(20) "
        "print(test:value()) "
        "test2 = mapper.Test.new() "
        "copied_test = test ",
        sol::script_pass_on_error);
    if (!result.valid()){
        sol::error err = result;
        std::cerr << "The code has failed to run!\n"
                  << err.what()
                  << std::endl;
        return false;
    }

    sol::object testobject = lua["copied_test"];
    if (testobject.is<Test&>()){
        auto& obj = testobject.as<Test&>();
        std::cout << "counter: " << obj.value() << std::endl;
    }
    if (!testobject.is<Test2&>()){
        std::cout << "copied_test is not a instance of Test2" << std::endl;
    }

    lua.script("numval = 100");
    sol::object numval = lua["numval"];
    std::cout << "numval: " << numval.as<std::string>() << std::endl;

    lua.script("array ={\"a\", \"b\", \"c\", abc=10, def=20}");
    sol::table array = lua["array"];
    std::cout << "arran size: " << array.size() <<std::endl;
    for (int i = 1; i <= array.size(); i++){
        std::cout << "array[" << i << "]: " << static_cast<sol::object>(array[i]).as<std::string>() << std::endl;
    }
    sol::object outofrange = array[4];
    if (outofrange.get_type() == sol::type::lua_nil){
        std::cout << "array[4] is null" <<std::endl;
    }

    {
        std::function<std::string (std::string)> func = std::bind(concat_string, "hogehoge", std::placeholders::_1);
        lua["concat"] = func;
        lua.script("print(concat(\"HOGEHOGE\"))");
    }
    lua.script("print(concat(\"HOGEHOGE\"))");

    {
        std::string str = "foobar";
        std::function<std::string (std::string)> func = [str](auto arg){
            return concat_string(str, arg);
        };
        lua["concat2"] = func;
        lua.script("print(concat2(\"FOOBAR\"))");
    }
    lua.script("print(concat2(\"FOOBAR\"))");

    std::cout << "########################################################" << std::endl;
    sol::object luafunc_o;
    lua["regfunc"] = [&luafunc_o](sol::object obj){
        luafunc_o = obj;
    };
    lua.script(
        "function luaf() print(\"OK OK OK\") end "
        "regfunc(luaf)"
    );
    if (luafunc_o.get_type() == sol::type::function){
        std::cout << "yes, luafunc_o is lua function" << std::endl;
        sol::protected_function f = luafunc_o;
        f();
    }

    return true;
}

static bool console_handler(MapperHandle mapper, MCONSOLE_MESSAGE_TYPE type, const char *msg, size_t len){
    std::cout.write(msg, len);
    return true;
}

static bool event_handler(MapperHandle mapper, MAPPER_EVENT ev, int64_t data){
    return true;
}

int main(int argc, char* argv[]){
    if(test()){
        return 0;
    }

    if (argc < 2){
        std::cerr << "usage: " << argv[0] << "script-path" << std::endl;
        return 1;
    }

    MapperHandle mapper = mapper_init(event_handler, console_handler, nullptr);
    mapper_run(mapper, argv[1]);

    return 0;
}