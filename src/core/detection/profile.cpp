#include "profile.hpp"
#include <filesystem>

namespace profile {
    const std::string rootPath = "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames";
    static logger log;

    bool userIdExists(const std::string& id) {
        fs::path fullPath = fs::path(rootPath) / id;

        if (!fs::exists(fullPath) || !fs::is_directory(fullPath)) {
            log.error("Profile '" + id + "' does not exist.");
            return false;
        }

        return true;
    }

    std::vector<std::string> detectUserIds() {
        std::vector<std::string> profileIds;

        if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
            log.fatal("Ubisoft savegames folder not found!");
            return profileIds;
        }

        for (const auto& entry : fs::directory_iterator(rootPath)) {
            if (fs::is_directory(entry.status())) {
                std::string dirname = entry.path().filename().string();
                if (dirname.length() == 36) {
                    profileIds.emplace_back(dirname);
                }
            }
        }

        return profileIds;
    }

}
