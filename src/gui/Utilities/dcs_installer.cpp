//
// dcs_installer.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "pch.h"
#include "dcs_installer.hpp"
#include <shlobj_core.h>
#include <vector>
#include <unordered_map>
#include <fstream>

static std::vector<std::string> tokens{
    "fsmapper", "=", "{", "}", ";", "fsmapper", ".", "basedir", "=",
};

struct parse_context{
    const std::string& target;
    int position_on_target{0};
    int matched_tokens{0};
    int matched_characters{0};
    bool inter_token{true};

    parse_context(const std::string& target): target(target){}

    void step_target(){
        position_on_target++;
    }

    int current_target_char(){
        if (position_on_target < target.length()){
            return static_cast<u_char>(target.at(position_on_target));
        }else{
            return -1;
        }
    }

    void step_dict(){
        matched_characters++;
        inter_token = false;
        auto& current_token = tokens[matched_tokens];
        if (current_token.length() <= matched_characters){
            matched_tokens++;
            matched_characters = 0;
            inter_token = true;
        }
    }

    int current_dict_char(){
        if (matched_tokens < tokens.size()){
            return static_cast<u_char>(tokens[matched_tokens].at(matched_characters));
        }else{
            return -1;
        }
    }
};

bool parse_front_section(parse_context& context){
    int current;
    for (;(current = context.current_target_char()) >= 0; context.step_target()){
        int current_dict = context.current_dict_char();
        if (current_dict < 0){
            return true;
        }
        if (context.inter_token && (current == ' ' || current == '\t')){
            // nothing to do
        }else if (current == current_dict){
            context.step_dict();
        }else{
            // syntax mismatch
            return false;
        }
    }
    return false;
}

bool parse_base_path(const char* input, std::string& output){
    static std::unordered_map<char, char> escape_dict{
        {'\'', '\''},
        {'"', '"'},
        {'\\', '\\'},
        {'a', '\a'},
        {'b', '\b'},
        {'f', '\f'},
        {'n', '\n'},
        {'r', '\r'},
        {'t', '\t'},
        {'v', '\v'},
    };
    bool is_befor_data{true};
    char quote{0};
    for (; *input; input++){
        if (is_befor_data){
            if (*input == ' ' || *input == '\t'){
                // skipping
            }else if (*input == '\'' || *input == '"'){
                is_befor_data = false;
                quote = *input;
            }else{
                return false;
            }
        }else{
            if (*input == quote){
                return true;
            }else if (*input == '\\'){
                // might be escaped charactor
                input++;
                if (escape_dict.count(*input) > 0){
                    output.push_back(escape_dict[*input]);
                }else{
                    return false;
                }
            }else{
                output.push_back(*input);
            }
        }
    }
    return false;
}

namespace dcs {
    installer::installer() : status(exporter_status::unknown){
        wchar_t* path;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path))){
            dcs_env_path = path;
            dcs_env_path = dcs_env_path / "Saved Games/DCS";
            export_lua_path = dcs_env_path / "Scripts/export.lua";
            CoTaskMemFree(path);
        }
        std::vector<char> buf;
        buf.resize(256);
        while(true){
            if (::GetModuleFileNameA(nullptr, &buf.at(0), static_cast<DWORD>(buf.size())) < buf.size()){
                break;
            }
            buf.resize(buf.size() + 256);
        }
        base_path = &buf.at(0);
        base_path.remove_filename();
    }

    bool installer::check(){
        exsisting_base_path.clear();
        if (dcs_env_path.string().length() == 0){
            status = exporter_status::unknown;
            return false;
        }else if (!std::filesystem::exists(dcs_env_path) || !std::filesystem::is_directory(dcs_env_path)){
            status = exporter_status::no_dcs;
        }else if (std::filesystem::exists(export_lua_path)){
            if (std::filesystem::is_regular_file(export_lua_path)){
                std::ifstream ifs{export_lua_path};
                if (!ifs){
                    status = exporter_status::failed_to_read;
                    return false;
                }
                auto num_exporters{0};
                std::string line;
                while (std::getline(ifs, line)){
                    parse_context context(line);
                    if (parse_front_section(context)){
                        num_exporters++;
                        std::string path;
                        if (parse_base_path(line.c_str() + context.position_on_target, path)) {
                            exsisting_base_path = path;
                        }
                    }else if (context.matched_tokens >= 5){
                        num_exporters++;
                    }
                }
                if (num_exporters == 0){
                    status = exporter_status::no_exporter;
                }else if (num_exporters == 1){
                    if (base_path == exsisting_base_path){
                        status = exporter_status::installed;
                    }else{
                        status = exporter_status::other_exporter;
                    }
                }else{
                    status = exporter_status::multiple_exporter;
                }
            }else{
                status = exporter_status::is_not_regular_file;
            }
        }else{
            status = exporter_status::no_file;
        }
        return true;
    }

    bool installer::install(){
        if (status == exporter_status::unknown){
            check();
        }
        if (status == exporter_status::unknown || status == exporter_status::no_dcs){
            return false;
        }
        auto parent_dir = export_lua_path.parent_path();
        if (!std::filesystem::exists(parent_dir)){
            try{
                std::filesystem::create_directories(parent_dir);
            }catch (std::filesystem::filesystem_error&){
                return false;
            }
        }

        auto new_export_lua_path = parent_dir / "export.lua.new";
        std::ofstream ofs(new_export_lua_path);
        if (!ofs){
            return false;
        }
        if (std::filesystem::exists(export_lua_path)){
            std::ifstream ifs(export_lua_path);
            if (!ifs){
                return false;
            }
            std::string line;
            while (std::getline(ifs, line)){
                parse_context context(line);
                parse_front_section(context);
                if (context.matched_tokens < 5){
                    ofs << line << std::endl;
                }
            }
        }

        static std::unordered_map<char, const char*> escape_dict{
            {'\'', "\\\'"},
            {'"', "\\\""},
            {'\\', "\\\\"},
            {'\a', "\\a"},
            {'\b', "\\b"},
            {'\f', "\\f"},
            {'\n', "\\n"},
            {'\r', "\\r"},
            {'\t', "\\t"},
            {'\v', "\\v"},
        };
        std::string escaped_base_dir;
        for (auto c : base_path.string()){
            if (escape_dict.count(c) > 0){
                auto ec = escape_dict[c];
                escaped_base_dir.append(ec);
            }else{
                escaped_base_dir.push_back(c);
            }
        }
        ofs << "fsmapper={};fsmapper.basedir='" << escaped_base_dir << "';dofile(fsmapper.basedir..'dcs-exporter/fsmapper.lua')" << std::endl;
        ofs.close();

        try{
            std::filesystem::rename(new_export_lua_path, export_lua_path);
        }catch (std::filesystem::filesystem_error&){
            return false;
        }

        return true;
    }
}