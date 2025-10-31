#pragma once
#include "../includes.hpp"
#include "imgui.h"

class UIManager {
  public:
    int selectedGameIndex;

    UIManager();
    ~UIManager();

    void Render(ImGuiWindowFlags window_flags);
};
