#include "Extension/GSICreator.h"
#include <filesystem>
#include <iostream>
#include <optional>
#include <vector>

namespace fs = std::filesystem;

std::optional<fs::path> GSCreator::findCS2CFG() {
    std::vector<fs::path> roots = {
        "C:/Program Files (x86)/Steam/steamapps/common",
        "D:/SteamLibrary/steamapps/common"
    };

    for (const auto& root : roots) {
        fs::path full = root / "Counter-Strike Global Offensive/game/csgo/cfg";

        if (fs::exists(full)) {
            return full;
        }
    }

    return std::nullopt;
}

void GSCreator::createConfig(const fs::path& path) {
    try {
        fs::copy_file(
            "Extension/gamestate_integration_test.cfg",
            path / "gamestate_integration_test.cfg",
            fs::copy_options::overwrite_existing
        );

        std::cout << "Config created at: " << path << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}