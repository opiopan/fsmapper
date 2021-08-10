//
// tools.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//


#include <map>
#include <vector>
#include <string_view>
#include <algorithm>
#include "tools.h"

//============================================================================================
// Convert webcolor string to Windows COLORREF
//============================================================================================
static std::map<std::string_view, COLORREF> webcolors = {
    {"lavender", 0xfae6e6},
    {"thistle", 0xd8bfd8},
    {"plum", 0xdda0dd},
    {"violet", 0xee82ee},
    {"orchid", 0xd670da},
    {"fuchsia", 0xff00ff},
    {"magenta", 0xff00ff},
    {"mediumorchid", 0xd355ba},
    {"mediumpurple", 0xdb7093},
    {"blueviolet", 0xe22b8a},
    {"darkviolet", 0xd30094},
    {"darkorchid", 0xcc3299},
    {"darkmagenta", 0x8b008b},
    {"purple", 0x800080},
    {"indigo", 0x82004b},
    {"darkslateblue", 0x8b3d48},
    {"slateblue", 0xcd5a6a},
    {"mediumslateblue", 0xee687b},
    {"pink", 0xcbc0ff},
    {"lightpink", 0xc1b6ff},
    {"hotpink", 0xb469ff},
    {"deeppink", 0x9314ff},
    {"palevioletred", 0x9370db},
    {"mediumvioletred", 0x8515c7},
    {"lightsalmon", 0x7aa0ff},
    {"salmon", 0x7280fa},
    {"darksalmon", 0x7a96e9},
    {"lightcoral", 0x8080f0},
    {"indianred", 0x5c5ccd},
    {"crimson", 0x3c14dc},
    {"firebrick", 0x2222b2},
    {"darkred", 0x00008b},
    {"red", 0x0000ff},
    {"orangered", 0x0045ff},
    {"tomato", 0x4763ff},
    {"coral", 0x507fff},
    {"darkorange", 0x008cff},
    {"orange", 0x00a5ff},
    {"yellow", 0x00ffff},
    {"lightyellow", 0xe0ffff},
    {"lemonchiffon", 0xcdfaff},
    {"lightgoldenrodyellow", 0xd2fafa},
    {"papayawhip", 0xd5efff},
    {"moccasin", 0xb5e4ff},
    {"peachpuff", 0xb9daff},
    {"palegoldenrod", 0xaae8ee},
    {"khaki", 0x8ce6f0},
    {"darkkhaki", 0x6bb7bd},
    {"gold", 0x00d7ff},
    {"cornsilk", 0xdcf8ff},
    {"blanchedalmond", 0xcdebff},
    {"bisque", 0xc4e4ff},
    {"navajowhite", 0xaddeff},
    {"wheat", 0xb3def5},
    {"burlywood", 0x87b8de},
    {"tan", 0x8cb4d2},
    {"rosybrown", 0x8f8fbc},
    {"sandybrown", 0x60a4f4},
    {"goldenrod", 0x20a5da},
    {"darkgoldenrod", 0x0b86b8},
    {"peru", 0x3f85cd},
    {"chocolate", 0x1e69d2},
    {"saddlebrown", 0x13458b},
    {"sienna", 0x2d52a0},
    {"brown", 0x2a2aa5},
    {"maroon", 0x000080},
    {"darkolivegreen", 0x2f6b55},
    {"olive", 0x008080},
    {"olivedrab", 0x238e6b},
    {"yellowgreen", 0x32cd9a},
    {"limegreen", 0x32cd32},
    {"lime", 0x00ff00},
    {"lawngreen", 0x00fc7c},
    {"chartreuse", 0x00ff7f},
    {"greenyellow", 0x2fffad},
    {"springgreen", 0x7fff00},
    {"mediumspringgreen", 0x9afa00},
    {"lightgreen", 0x90ee90},
    {"palegreen", 0x98fb98},
    {"darkseagreen", 0x8fbc8f},
    {"mediumseagreen", 0x71b33c},
    {"seagreen", 0x578b2e},
    {"forestgreen", 0x228b22},
    {"green", 0x008000},
    {"darkgreen", 0x006400},
    {"mediumaquamarine", 0xaacd66},
    {"aqua", 0xffff00},
    {"cyan", 0xffff00},
    {"lightcyan", 0xffffe0},
    {"paleturquoise", 0xeeeeaf},
    {"aquamarine", 0xd4ff7f},
    {"turquoise", 0xd0e040},
    {"mediumturquoise", 0xccd148},
    {"darkturquoise", 0xd1ce00},
    {"lightseagreen", 0xaab220},
    {"cadetblue", 0xa09e5f},
    {"darkcyan", 0x8b8b00},
    {"teal", 0x808000},
    {"lightsteelblue", 0xdec4b0},
    {"powderblue", 0xe6e0b0},
    {"lightblue", 0xe6d8ad},
    {"skyblue", 0xebce87},
    {"lightskyblue", 0xface87},
    {"deepskyblue", 0xffbf00},
    {"dodgerblue", 0xff901e},
    {"cornflowerblue", 0xed9564},
    {"steelblue", 0xb48246},
    {"royalblue", 0xe16941},
    {"blue", 0xff0000},
    {"mediumblue", 0xcd0000},
    {"darkblue", 0x8b0000},
    {"navy", 0x800000},
    {"midnightblue", 0x701919},
    {"white", 0xffffff},
    {"snow", 0xfafaff},
    {"honeydew", 0xf0fff0},
    {"mintcream", 0xfafff5},
    {"azure", 0xfffff0},
    {"aliceblue", 0xfff8f0},
    {"ghostwhite", 0xfff8f8},
    {"whitesmoke", 0xf5f5f5},
    {"seashell", 0xeef5ff},
    {"beige", 0xdcf5f5},
    {"oldlace", 0xe6f5fd},
    {"floralwhite", 0xf0faff},
    {"ivory", 0xf0ffff},
    {"antiquewhite", 0xd7ebfa},
    {"linen", 0xe6f0fa},
    {"lavenderblush", 0xf5f0ff},
    {"mistyrose", 0xe1e4ff},
    {"gainsboro", 0xdcdcdc},
    {"lightgray", 0xd3d3d3},
    {"silver", 0xc0c0c0},
    {"darkgray", 0xa9a9a9},
    {"gray", 0x808080},
    {"dimgray", 0x696969},
    {"lightslategray", 0x998877},
    {"slategray", 0x908070},
    {"darkslategray", 0x4f4f2f},
    {"black", 0x000000},
};



std::optional<COLORREF> webcolor_to_colorref(const std::string& color_str){
    auto hex = [](int c) -> std::optional<int> {
        if (c >= '0' && c<= '9'){
            return c - '0';
        }else if (c >= 'a' && c <= 'f'){
            return c - 'a' + 10;
        }else if (c >= 'A' && c <= 'F'){
            return c - 'A' + 10;
        }else{
            return std::nullopt;
        }
    };

    if (color_str.length() == 7 && color_str[0] == '#'){
        static const std::vector<int> seq_24bit = {2, 1, 4, 3, 6, 5};
        int value = 0;
        for (auto pos : seq_24bit){
            auto part = hex(color_str[pos]);
            if (part){
                value <<= 4;
                value |= *part;
            }else{
                return std::nullopt;
            }
        }
        return value;
    }else{
        std::string normalized_name = color_str;
        std::transform(
            normalized_name.begin(), normalized_name.end(), normalized_name.begin(),
            [](unsigned char c){return std::tolower(c);});
        if (webcolors.count(normalized_name) > 0){
            return webcolors.at(normalized_name);
        }else{
            return std::nullopt;
        }
    }
}