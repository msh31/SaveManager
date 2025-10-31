#pragma once
#include "../savemanager.hpp"

namespace profile {
    std::vector<std::string> detectUserIds();
    bool userIdExists(const std::string& id);
}
