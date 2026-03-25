#pragma once
#include "core/detection/idetector.hpp"

class CustomDetector : public IDetector {
public:
    void find_saves(const fs::path& prefix, std::vector<Game>& out_games) const override;
};
