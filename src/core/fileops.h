//
// fileops.h
//  Author: Hiroshi Murayama <opiopan@gmail.com>
//

#pragma once

#include <optional>
#include <filesystem>

namespace fileops{
    std::optional<std::filesystem::path> find_file_in_paths(const char* search_paths, const char* subpath);
}