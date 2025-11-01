#pragma once
#include "../includes.hpp"
#include "imgui.h"

class UIManager {
  public:
    int selectedGameIndex;

    UIManager(Config& cfg);
    ~UIManager();

    void Render(ImGuiWindowFlags window_flags);

  private:
    Config& config;
};
