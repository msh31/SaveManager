#pragma once
#include "../includes.hpp"

class UIManager {
  public:
    int currentTab;
    int selectedGameIndex;

    UIManager();
    ~UIManager();

    void Render(ImGuiWindowFlags window_flags);
};
