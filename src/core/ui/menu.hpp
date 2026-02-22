#pragma once

#include <string>
#include <vector>
#include <functional>
#include "../../detection/detection.hpp"

struct MenuItem {
    std::string label;
    std::function<void(const Detection::DetectionResult&)> action;
    bool is_exit;
};

class Menu {
public:
    Menu(const std::string& title = "");

    void add_item(const std::string& label,
                  std::function<void(const Detection::DetectionResult&)> action);
    void add_exit_item(const std::string& label = "Quit");

    bool run(const Detection::DetectionResult& context);

private:
    void display() const;
    int get_selection() const;

    std::string title;
    std::vector<MenuItem> items;
};
