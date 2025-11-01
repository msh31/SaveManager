#pragma once
#include "../includes.hpp"
#include "imgui.h"

class UIManager {
  public:
    int selectedGameIndex;
    int selectedProfileIndex;
    std::string getSelectedProfile();
    std::vector<std::string> detectedProfiles;
    bool needsProfileSelection = false;
    bool hasValidSelection();

    UIManager();
    ~UIManager();

    void Render(ImGuiWindowFlags window_flags);
};
