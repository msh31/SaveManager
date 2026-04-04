#pragma once
#include "backend/detection/detection.hpp"

class IDetector {
public:
    virtual ~IDetector() {};

    virtual void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const = 0;
};
