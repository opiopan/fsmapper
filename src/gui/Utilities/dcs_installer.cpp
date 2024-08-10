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
#include <format>
#include "App.xaml.h"

using App = winrt::gui::implementation::App;

//============================================================================================
// DCS Export.lua parser
//============================================================================================
static std::vector<std::string> tokens{
    "fsmapper", "=", "{", "basedir", "=",
};
static constexpr auto minimum_token_num = 3;

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

//============================================================================================
// Installing / Uninstalling the exporter
//============================================================================================
namespace dcs {
    installer::installer() : status(exporter_status::unknown){
        wchar_t* path;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, NULL, &path))){
            dcs_env_path = path;
            dcs_env_path = dcs_env_path / "Saved Games" / "DCS";
            export_lua_path = dcs_env_path / "Scripts" / "Export.lua";
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
        existing_base_path.clear();
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
                            existing_base_path = path;
                            existing_base_path.make_preferred();

                        }
                    }else if (context.matched_tokens >= minimum_token_num){
                        num_exporters++;
                    }
                }
                if (num_exporters == 0){
                    status = exporter_status::no_exporter;
                }else if (num_exporters == 1){
                    if (base_path == existing_base_path){
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

    bool installer::install(mode mode){
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
                if (context.matched_tokens < minimum_token_num){
                    ofs << line << std::endl;
                }
            }
        }

        if (mode == mode::install){
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
            for (auto c : base_path.generic_string()){
                if (escape_dict.count(c) > 0){
                    auto ec = escape_dict[c];
                    escaped_base_dir.append(ec);
                }else{
                    escaped_base_dir.push_back(c);
                }
            }
            ofs << "fsmapper={basedir='" << escaped_base_dir << "'};dofile(fsmapper.basedir..'dcs-exporter/fsmapper.lua')" << std::endl;
        }

        ofs.close();
        try{
            std::filesystem::rename(new_export_lua_path, export_lua_path);
        }catch (std::filesystem::filesystem_error&){
            return false;
        }

        return true;
    }

    winrt::Windows::Foundation::IAsyncAction installer::show_install_error(){
        winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog;
        dialog.Title(winrt::box_value(L"File Access Error"));
        std::wstring message = std::format(
            L"Failed to update the 'export.lua' file for DCS World. Please check the file below for any issues.\n"
            "\t{}", export_lua_path.generic_wstring());
        dialog.Content(winrt::box_value(winrt::hstring(message)));
        dialog.PrimaryButtonText(L"OK");
        dialog.XamlRoot(App::TopWindow().Content().XamlRoot());
        co_await dialog.ShowAsync();
    }
}

//============================================================================================
// Content Dialog to confirm install or uninstall
//============================================================================================
namespace dcs{
    winrt::Windows::Foundation::IAsyncOperation<int> confirm_change_export_lua(fsmapper::config::dcs_exporter_mode next_mode, installer& status){
        struct message_def{
            std::wstring (*message)(const wchar_t*, const wchar_t*);
            int primary_value{confirmation_yes};
            int secondary_value{confirmation_no};
            const wchar_t* primary_button{L"Yes"};
            const wchar_t* secondary_button{L"No"};
        };
        auto confirm_on_start_up = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(
                L"DCS World is detected. Do you want to register the integration with fsmapper in the 'exporter.lua' file of DCS World?\n\n"
                "You can also register later from the Settings page even if you select ‘No’.",
                v1, v2);
        };
        auto confirm_on_switch_to_on = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(
                L"Do you want to register the integration with fsmapper in the 'exporter.lua' file of DCS World?",
                v1, v2);
        };
        auto confirm_on_switch_to_off = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(
                L"Do you want to remove the fsmapper integration from 'export.lua' file of DCS World?\n\n"
                "If you select ‘No’, the integration will only be turned off on the fsmapper side.",
                v1, v2);
        };
        auto confirm_other_install = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(
                L"Another fsmapper from the followoing directory is registered in DCS World. Do you want to re-register this fsmapper?\n\n"
                "\t{1}",
                v1, v2);
        };
        auto confirm_multiple_definition = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(                
                L"It looks like there are multiple fsmapper integration module entries in the 'export.lua' file of DCS World.\n"
                "Do you want to remove these and register the correct fsmapper integration module?\n\n"
                "You can choose ‘No’ and check the following file yourself if you prefer.\n\n"
                "\t{0}",
                v1, v2);
        };
        auto confirm_multiple_definition_on_switch_to_off = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(
                L"It looks like there are multiple fsmapper integration module entries in the 'export.lua' file of DCS World.¥n"
                "Do you want to remove all of these?\n\n"
                "You can choose ‘No’ and check the following file yourself if you prefer. "
                "In this case, the integration will only be turned off on the fsmapper side.\n\n"
                "\t{0}",
                v1, v2);
        };
        auto confirm_cannot_read = [](const wchar_t* v1, const wchar_t* v2){
            return std::format(
                L"Failed to read the 'export.lua' file of DCS World. Please ensure you can access the file below.\n\n"
                "\t{0}",
                v1, v2);
        };
        static std::unordered_map<fsmapper::config::dcs_exporter_mode, std::unordered_map<exporter_status, message_def>> definition{
            {fsmapper::config::dcs_exporter_mode::unknown, {
                {exporter_status::unknown, {nullptr, confirmation_unknown}},
                {exporter_status::no_dcs, {nullptr, confirmation_unknown}},
                {exporter_status::no_file, {confirm_on_start_up}},
                {exporter_status::is_not_regular_file, {confirm_cannot_read, confirmation_no, confirmation_no, L"OK", nullptr}},
                {exporter_status::failed_to_read, {confirm_cannot_read, confirmation_no, confirmation_no, L"OK", nullptr}},
                {exporter_status::no_exporter, {confirm_on_start_up}},
                {exporter_status::multiple_exporter, {confirm_multiple_definition, confirmation_yes, no_changes_needed}},
                {exporter_status::other_exporter, {confirm_other_install, confirmation_yes, no_changes_needed}},
                {exporter_status::installed, {nullptr, no_changes_needed}},
            }},
            {fsmapper::config::dcs_exporter_mode::on, {
                {exporter_status::unknown, {nullptr, confirmation_no}},
                {exporter_status::no_dcs, {nullptr, confirmation_no}},
                {exporter_status::no_file, {confirm_on_switch_to_on}},
                {exporter_status::is_not_regular_file, {confirm_cannot_read, confirmation_no, confirmation_no, L"OK", nullptr}},
                {exporter_status::failed_to_read, {confirm_cannot_read, confirmation_no, confirmation_no, L"OK", nullptr}},
                {exporter_status::no_exporter, {confirm_on_switch_to_on}},
                {exporter_status::multiple_exporter, {confirm_multiple_definition}},
                {exporter_status::other_exporter, {confirm_other_install}},
                {exporter_status::installed, {nullptr, no_changes_needed}},
            }},
            {fsmapper::config::dcs_exporter_mode::off, {
                {exporter_status::unknown, {confirm_cannot_read, no_changes_needed, confirmation_no, L"OK", nullptr}},
                {exporter_status::no_dcs, {nullptr, no_changes_needed}},
                {exporter_status::no_file, {nullptr, no_changes_needed}},
                {exporter_status::is_not_regular_file, {confirm_cannot_read, no_changes_needed, confirmation_no, L"OK", nullptr}},
                {exporter_status::failed_to_read, {confirm_cannot_read, no_changes_needed, confirmation_no, L"OK", nullptr}},
                {exporter_status::no_exporter, {nullptr, no_changes_needed}},
                {exporter_status::multiple_exporter, {confirm_multiple_definition_on_switch_to_off, confirmation_yes, no_changes_needed}},
                {exporter_status::other_exporter, {confirm_on_switch_to_off, confirmation_yes, no_changes_needed}},
                {exporter_status::installed, {confirm_on_switch_to_off, confirmation_yes, no_changes_needed}},
            }},
        };

        auto& condition = definition[next_mode][status.status];

        if (condition.message){
            winrt::Microsoft::UI::Xaml::Controls::ContentDialog dialog;
            dialog.Title(winrt::box_value(L"DCS World integration function"));
            std::wstring message = 
                condition.message(status.export_lua_path.wstring().c_str(), status.existing_base_path.wstring().c_str());
            dialog.Content(winrt::box_value(winrt::hstring(message)));
            dialog.PrimaryButtonText(condition.primary_button);
            if (condition.secondary_button){
                dialog.CloseButtonText(condition.secondary_button);
            }
            dialog.XamlRoot(App::TopWindow().Content().XamlRoot());
            auto result =  co_await dialog.ShowAsync();

            if (result == winrt::Microsoft::UI::Xaml::Controls::ContentDialogResult::Primary){
                co_return condition.primary_value;
            }else{
                co_return condition.secondary_value;
            }
        }else{
            co_return condition.primary_value;
        }
    }
}