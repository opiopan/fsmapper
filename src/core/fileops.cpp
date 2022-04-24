//
// fileops.cpp
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#include "fileops.h"

#include <memory>
#include <filesystem>

namespace fileops{
    std::optional<std::filesystem::path> find_file_in_paths(const char* search_paths, const char* subpath){
        // in case of absolute path
        std::filesystem::path sub{subpath};
        if (sub.is_absolute()){
            if (std::filesystem::exists(sub)){
                return sub;
            }else{
                return std::nullopt;
            }
        }

        // in case of relative path
        size_t top = 0;
        size_t end = 0;
        do {
            if (search_paths[end] == ';' || search_paths[end] == '\0'){
                if (end > top){
                    std::string prefix{search_paths + top, end - top};
                    std::filesystem::path path{prefix};
                    path /= subpath;
                    if (std::filesystem::exists(path)){
                        return path;
                    }
                }
                top = end + 1;
            }
            end++;
        }while (search_paths[end - 1]);
        
        // not find a specified file in search paths
        return std::nullopt;
    }
}