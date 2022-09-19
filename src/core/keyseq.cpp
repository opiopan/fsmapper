//
// keyseq.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "keyseq.h"

#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
#include "engine.h"
#include "simplewindow.h"

//============================================================================================
// translate string to virtual key code
//============================================================================================
static std::unordered_map<std::string, int> keycode_map{
    {"VK_LBUTTON", 0x01},
    {"VK_RBUTTON", 0x02},
    {"VK_CANCEL", 0x03},
    {"VK_MBUTTON", 0x04},
    {"VK_XBUTTON1", 0x05},
    {"VK_XBUTTON2", 0x06},
    {"VK_BACK", 0x08},
    {"VK_TAB", 0x09},
    {"VK_CLEAR", 0x0C},
    {"VK_RETURN", 0x0D},
    {"VK_SHIFT", 0x10},
    {"VK_CONTROL", 0x11},
    {"VK_MENU", 0x12},
    {"VK_PAUSE", 0x13},
    {"VK_CAPITAL", 0x14},
    {"VK_KANA", 0x15},
    {"VK_HANGUEL", 0x15},
    {"VK_HANGUL", 0x15},
    {"VK_IME_ON", 0x16},
    {"VK_JUNJA", 0x17},
    {"VK_FINAL", 0x18},
    {"VK_HANJA", 0x19},
    {"VK_KANJI", 0x19},
    {"VK_IME_OFF", 0x1A},
    {"VK_ESCAPE", 0x1B},
    {"VK_CONVERT", 0x1C},
    {"VK_NONCONVERT", 0x1D},
    {"VK_ACCEPT", 0x1E},
    {"VK_MODECHANGE", 0x1F},
    {"VK_SPACE", 0x20},
    {"VK_PRIOR", 0x21},
    {"VK_NEXT", 0x22},
    {"VK_END", 0x23},
    {"VK_HOME", 0x24},
    {"VK_LEFT", 0x25},
    {"VK_UP", 0x26},
    {"VK_RIGHT", 0x27},
    {"VK_DOWN", 0x28},
    {"VK_SELECT", 0x29},
    {"VK_PRINT", 0x2A},
    {"VK_EXECUTE", 0x2B},
    {"VK_SNAPSHOT", 0x2C},
    {"VK_INSERT", 0x2D},
    {"VK_DELETE", 0x2E},
    {"VK_HELP", 0x2F},
    {"0", 0x30},
    {"1", 0x31},
    {"2", 0x32},
    {"3", 0x33},
    {"4", 0x34},
    {"5", 0x35},
    {"6", 0x36},
    {"7", 0x37},
    {"8", 0x38},
    {"9", 0x39},
    {"A", 0x41},
    {"B", 0x42},
    {"C", 0x43},
    {"D", 0x44},
    {"E", 0x45},
    {"F", 0x46},
    {"G", 0x47},
    {"H", 0x48},
    {"I", 0x49},
    {"J", 0x4A},
    {"K", 0x4B},
    {"L", 0x4C},
    {"M", 0x4D},
    {"N", 0x4E},
    {"O", 0x4F},
    {"P", 0x50},
    {"Q", 0x51},
    {"R", 0x52},
    {"S", 0x53},
    {"T", 0x54},
    {"U", 0x55},
    {"V", 0x56},
    {"W", 0x57},
    {"X", 0x58},
    {"Y", 0x59},
    {"Z", 0x5A},
    {"VK_LWIN", 0x5B},
    {"VK_RWIN", 0x5C},
    {"VK_APPS", 0x5D},
    {"VK_SLEEP", 0x5F},
    {"VK_NUMPAD0", 0x60},
    {"VK_NUMPAD1", 0x61},
    {"VK_NUMPAD2", 0x62},
    {"VK_NUMPAD3", 0x63},
    {"VK_NUMPAD4", 0x64},
    {"VK_NUMPAD5", 0x65},
    {"VK_NUMPAD6", 0x66},
    {"VK_NUMPAD7", 0x67},
    {"VK_NUMPAD8", 0x68},
    {"VK_NUMPAD9", 0x69},
    {"VK_MULTIPLY", 0x6A},
    {"VK_ADD", 0x6B},
    {"VK_SEPARATOR", 0x6C},
    {"VK_SUBTRACT", 0x6D},
    {"VK_DECIMAL", 0x6E},
    {"VK_DIVIDE", 0x6F},
    {"VK_F1", 0x70},
    {"VK_F2", 0x71},
    {"VK_F3", 0x72},
    {"VK_F4", 0x73},
    {"VK_F5", 0x74},
    {"VK_F6", 0x75},
    {"VK_F7", 0x76},
    {"VK_F8", 0x77},
    {"VK_F9", 0x78},
    {"VK_F10", 0x79},
    {"VK_F11", 0x7A},
    {"VK_F12", 0x7B},
    {"VK_F13", 0x7C},
    {"VK_F14", 0x7D},
    {"VK_F15", 0x7E},
    {"VK_F16", 0x7F},
    {"VK_F17", 0x80},
    {"VK_F18", 0x81},
    {"VK_F19", 0x82},
    {"VK_F20", 0x83},
    {"VK_F21", 0x84},
    {"VK_F22", 0x85},
    {"VK_F23", 0x86},
    {"VK_F24", 0x87},
    {"VK_NUMLOCK", 0x90},
    {"VK_SCROLL", 0x91},
    {"VK_LSHIFT", 0xA0},
    {"VK_RSHIFT", 0xA1},
    {"VK_LCONTROL", 0xA2},
    {"VK_RCONTROL", 0xA3},
    {"VK_LMENU", 0xA4},
    {"VK_RMENU", 0xA5},
    {"VK_BROWSER_BACK", 0xA6},
    {"VK_BROWSER_FORWARD", 0xA7},
    {"VK_BROWSER_REFRESH", 0xA8},
    {"VK_BROWSER_STOP", 0xA9},
    {"VK_BROWSER_SEARCH", 0xAA},
    {"VK_BROWSER_FAVORITES", 0xAB},
    {"VK_BROWSER_HOME", 0xAC},
    {"VK_VOLUME_MUTE", 0xAD},
    {"VK_VOLUME_DOWN", 0xAE},
    {"VK_VOLUME_UP", 0xAF},
    {"VK_MEDIA_NEXT_TRACK", 0xB0},
    {"VK_MEDIA_PREV_TRACK", 0xB1},
    {"VK_MEDIA_STOP", 0xB2},
    {"VK_MEDIA_PLAY_PAUSE", 0xB3},
    {"VK_LAUNCH_MAIL", 0xB4},
    {"VK_LAUNCH_MEDIA_SELECT", 0xB5},
    {"VK_LAUNCH_APP1", 0xB6},
    {"VK_LAUNCH_APP2", 0xB7},
    {"VK_OEM_1", 0xBA},
    {"VK_OEM_PLUS", 0xBB},
    {"VK_OEM_COMMA", 0xBC},
    {"VK_OEM_MINUS", 0xBD},
    {"VK_OEM_PERIOD", 0xBE},
    {"VK_OEM_2", 0xBF},
    {"VK_OEM_3", 0xC0},
    {"VK_OEM_4", 0xDB},
    {"VK_OEM_5", 0xDC},
    {"VK_OEM_6", 0xDD},
    {"VK_OEM_7", 0xDE},
    {"VK_OEM_8", 0xDF},
    {"VK_OEM_102", 0xE2},
    {"VK_PROCESSKEY", 0xE5},
    {"VK_PACKET", 0xE7},
    {"VK_ATTN", 0xF6},
    {"VK_CRSEL", 0xF7},
    {"VK_EXSEL", 0xF8},
    {"VK_EREOF", 0xF9},
    {"VK_PLAY", 0xFA},
    {"VK_ZOOM", 0xFB},
    {"VK_NONAME", 0xFC},
    {"VK_PA1", 0xFD},
    {"VK_OEM_CLEAR", 0xFE},
};

static int translate_to_keycode(sol::object obj){
    if (obj.get_type() == sol::type::number){
        auto value = round(obj.as<double>());
        if (value < 1 || value > 0xFE){
            std::ostringstream os;
            os << "specified value, " << value << ", is out of range of virtual key code";
            throw MapperException(os.str());
        }
        return value;
    }else if (obj.get_type() == sol::type::string){
        std::string&& name = obj.as<std::string>();
        std::transform(
            name.begin(), name.end(), name.begin(),
            [](unsigned char c){return std::toupper(c);});
        if (keycode_map.count(name) > 0){
            return keycode_map[name];
        }
        std::string new_name{"VK_"};
        new_name += name;
        if (keycode_map.count(new_name) > 0){
            return keycode_map[new_name];
        }
        std::ostringstream os;
        os << "specified string \"" << name << "\" is invalid virtual key code name";
        throw MapperException(os.str());
    }else{
        throw MapperException("virtual key code must be specified as string or number");
    }
}

static inline bool is_able_to_use_modifier(int code){
    return code == VK_SHIFT || code == VK_LSHIFT || code == VK_RSHIFT ||
           code == VK_CONTROL || code == VK_LCONTROL || code == VK_RCONTROL ||
           code == VK_MENU || VK_LMENU || VK_RMENU ||
           code == VK_LWIN || VK_RWIN;
}

//============================================================================================
// Key sequence object definition
//============================================================================================
namespace keyseq{
    class key_sequence{
        unsigned int size;
        std::unique_ptr<INPUT[]> data;
        int duration {100};
        int interval {0};

    public:
        key_sequence() = delete;

        key_sequence(sol::object object){
            if (object.get_type() != sol::type::table){
                throw MapperException("function argument must be a table");
            }
            sol::table params = object;
            sol::object codes_object = params["codes"];
            if (codes_object.get_type() != sol::type::table){
                throw MapperException("'codes' parameter must be specified as an array");
            }
            sol::table codes = codes_object;
            sol::object modifiers_object = params["modifiers"];
            auto offset = 0;
            auto extra_info = ::GetMessageExtraInfo();
            if (modifiers_object.get_type() == sol::type::table){
                sol::table modifiers = modifiers_object;
                size = codes.size() * 2 +  modifiers.size() * 2;
                data = std::move(std::make_unique<INPUT[]>(size));
                offset = modifiers.size();
                for (auto i = 0; i < modifiers.size(); i++){
                    auto code = translate_to_keycode(modifiers[i + 1]);
                    if (!is_able_to_use_modifier(code)){
                        throw MapperException("virtual key code which cannot be used as modifier is specified");
                    }
                    auto entry = data.get() + i;
                    entry->type = INPUT_KEYBOARD;
                    entry->ki.wVk = 0;
                    entry->ki.wScan = ::MapVirtualKey(code, 0);
                    entry->ki.time = 0;
                    entry->ki.dwExtraInfo = extra_info;
                    entry->ki.dwFlags = KEYEVENTF_SCANCODE;
                }
                for (auto i = 0; i < modifiers.size(); i++){
                    auto src = data.get() + i;
                    auto dest = data.get() + size - 1 - i;
                    *dest = *src;
                    dest->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
                }
            }else{
                size = codes.size() * 2;
                data = std::move(std::make_unique<INPUT[]>(size));
            }
            for (auto i = 0; i < codes.size(); i++){
                auto on = data.get() + offset + i * 2;
                auto off = on + 1;
                auto code = translate_to_keycode(codes[i + 1]);
                on->type = INPUT_KEYBOARD;
                on->ki.wVk = 0;
                on->ki.wScan = ::MapVirtualKeyA(code, 0);
                on->ki.time = 0;
                on->ki.dwExtraInfo = extra_info;
                on->ki.dwFlags = KEYEVENTF_SCANCODE;
                *off = *on;
                off->ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
            }
        }

        key_sequence(const key_sequence& srca, const key_sequence& srcb){
            size = srca.size + srcb.size;
            data = std::make_unique<INPUT[]>(size);
            memcpy(data.get(), srca.data.get(), sizeof(INPUT) * srca.size);
            memcpy(data.get() + srca.size, srcb.data.get(), sizeof(INPUT) * srcb.size);
        }

        key_sequence(const key_sequence& src){
            *this = src;
        }

        key_sequence(key_sequence&& src){
            *this = std::move(src);
        }

        key_sequence& operator = (const key_sequence& src){
            size = src.size;
            data = std::make_unique<INPUT[]>(size);
            memcpy(data.get(), src.data.get(), sizeof(INPUT) * size);
            return *this;
        }

        key_sequence& operator = (key_sequence&& src){
            size = src.size;
            data = std::move(src.data);
            return *this;
        }

        inline int get_duration(){return duration;}
        inline int get_interval(){return interval;}

        void emulate(){
            auto next = proceed(0);
            if (next < size){
                auto context = std::make_shared<key_sequence>(*this);
                context->delay(context, next);
            }
        }

        std::shared_ptr<NativeAction::Function> emulator(){
            auto context = std::make_shared<key_sequence>(*this);

            NativeAction::Function::ACTION_FUNCTION func = [context](Event&, sol::state&){
                auto next = context->proceed(0);
                context->delay(context, next);
            };
            return std::make_shared<NativeAction::Function>("mapper.key_sequence:emulate()", func);
        }

    protected:
        unsigned int proceed(unsigned int from){
            if (from < size){
                auto polarity = data.get()[from].ki.dwFlags & KEYEVENTF_KEYUP;
                auto to = from;
                for (; to < size; to++){
                    if ((data.get()[to].ki.dwFlags & KEYEVENTF_KEYUP) != polarity){
                        break;
                    }
                }
                ::SendInput(to - from, data.get() + from, sizeof(INPUT));
                return to;
            }else{
                return size;
            }
        }

        void delay(std::shared_ptr<key_sequence>object, unsigned int from){
            if (from < object->size){
                auto delayed_time = data.get()[from].ki.dwFlags & KEYEVENTF_KEYUP ? object->interval : object->duration;
                NativeAction::Function::ACTION_FUNCTION logic = [object, from](Event&, sol::state&){
                    auto next = object->proceed(from);
                    object->delay(object, next);
                };
                auto function = std::make_shared<NativeAction::Function>("mapper.key_sequecne:emulate()", logic);
                auto action = std::make_shared<NativeAction>(function);
                Event ev(static_cast<int64_t>(EventID::NILL));
                mapper_EngineInstance()->invokeActionIn(action, ev, MapperEngine::MILLISEC(delayed_time));
            }
        }
    };
}

//============================================================================================
// create lua evironment
//============================================================================================
void keyseq::create_lua_env(MapperEngine& engine, sol::table& mapper_table){
    mapper_table.new_usertype<key_sequence>(
        "key_sequence",
        sol::call_constructor, sol::factories([&engine](sol::object arg){
            return lua_c_interface(engine, "key_sequence", [&arg]{
                return std::make_shared<key_sequence>(arg);
            });
        }),
        "emulate", &key_sequence::emulate,
        "emulator", &key_sequence::emulator
    );
}
