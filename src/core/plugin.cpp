//
// plugin.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "plugin.h"
#include "engine.h"

#include <filesystem>
#include <algorithm>
#include <sstream>

PluginManager::PluginManager(){
    auto engine = mapper_EngineInstance();
    auto list_modules = [this, engine](const std::string& plugin_folder){
        if (plugin_folder.empty()){
            return;
        }

        std::ostringstream os;
        os << "plugin: search for plugin modules under the following folder:\n    " << plugin_folder;
        engine->putLog(MCONSOLE_DEBUG, os.str().c_str());

        try{
            for (auto& file : std::filesystem::directory_iterator{plugin_folder}){
                auto ext = file.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), [](int c){return std::tolower(c);});
                if (ext == ".dll"){
                    auto module = ::LoadLibraryA(file.path().string().c_str());
                    if (module == nullptr){
                        std::ostringstream os;
                        os << "plugin: cannot load the plugin module " << file.path().filename();
                        engine->putLog(MCONSOLE_WARNING, os.str().c_str());
                        break;
                    }
                    auto proc = reinterpret_cast<MAPPER_PLUGIN_DEVICE_OPS* (*)()>(
                        ::GetProcAddress(module, "getMapperPluginDeviceOps")
                    );
                    auto ops = proc ? proc() : nullptr;
                    if (ops && ops->name && ops->init && ops->term && ops->open && ops->start &&
                        ops->close && ops->getUnitNum && ops->getUnitDef &&ops->sendUnitValue){
                        modules.emplace_back(file.path().filename().string(), module, ops);
                    }else{
                        std::ostringstream os;
                        os << "plugin: " << file.path().filename();
                        os << "does not complient with fsmapper plugin module interface rule, ";
                        os << "ignore this module";
                        engine->putLog(MCONSOLE_WARNING, os.str().c_str());
                        ::FreeLibrary(module);
                    }
                }
            }
        }catch (std::filesystem::filesystem_error&){
            std::ostringstream os;
            os << "plugin: an error occurred while accessing the plugin folder:\n    " << plugin_folder;
            engine->putLog(MCONSOLE_WARNING, os.str().c_str());
        }
    };

    list_modules(engine->getOptions().plugin_folder);
    list_modules(engine->getOptions().user_plugin_folder);
    list_modules(engine->getOptions().app_plugin_folder);

    if (modules.size() == 0){
        engine->putLog(MCONSOLE_DEBUG, "plugin: no plugin modules are found");
    }else{
        std::ostringstream os;
        os << "plugin: " << modules.size() << (modules.size() == 1 ? " plugin module is " : " plugin modules are ") << "found:";
        for (auto& module : modules){
            os << std::endl << "    " << module.ops->name << " : " << module.ops->description;
        }
        engine->putLog(MCONSOLE_DEBUG, os.str().c_str());
    }
}

PluginManager::~PluginManager(){
    for (auto& module : modules){
        ::FreeLibrary(module.module);
    }
}
