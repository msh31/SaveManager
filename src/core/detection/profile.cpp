#include <filesystem>
#include "profile.hpp"

#ifdef _WIN32
#define UBI_PROFILE_PATH "C:\\Program Files (x86)\\Ubisoft\\Ubisoft Game Launcher\\savegames" 
#elif defined(__linux__)
#define UBI_PROFILE_PATH "THIS IS A MANUAL ENTRY"
#else
#error "Unsupported OS"
#endif

namespace profile {
    static logger log;

    bool userIdExists(const std::string& id) {
        fs::path fullPath = fs::path(UBI_PROFILE_PATH) / id;

        if (!fs::exists(fullPath) || !fs::is_directory(fullPath)) {
            log.error("Profile '" + id + "' does not exist.");
            return false;
        }

        return true;
    }

    std::vector<std::string> detectUserIds() {
        std::vector<std::string> profileIds;

        if (!fs::exists(UBI_PROFILE_PATH) || !fs::is_directory(UBI_PROFILE_PATH)) {
            log.fatal("Ubisoft savegames folder not found!");
            return profileIds;
        }

        for (const auto& entry : fs::directory_iterator(UBI_PROFILE_PATH)) {
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
