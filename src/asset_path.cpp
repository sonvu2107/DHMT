#include "asset_path.h"

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

std::string resolveAssetPath(const std::string& relativePath) {
    std::vector<fs::path> candidates;

#ifdef _WIN32
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) != 0) {
        fs::path exeDir = fs::path(exePath).parent_path();
        candidates.push_back(exeDir / relativePath);
        candidates.push_back(exeDir / ".." / relativePath);
        candidates.push_back(exeDir / "../.." / relativePath);
    }
#endif

    candidates.emplace_back(relativePath);
    candidates.emplace_back(fs::path("..") / relativePath);
    candidates.emplace_back(fs::path("../..") / relativePath);

    for (const auto& candidate : candidates) {
        std::error_code ec;
        if (fs::exists(candidate, ec)) {
            return fs::absolute(candidate).string();
        }
    }

    return relativePath;
}
