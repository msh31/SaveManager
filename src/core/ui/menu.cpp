#include <iostream>

#include "menu.hpp"
#include "input_validator.hpp"
#include "../../helpers/utils.hpp"

Menu::Menu(const std::string& title) : title(title) {}

void Menu::add_item(const std::string& label,
                    std::function<void(const Detection::DetectionResult&)> action) {
    items.push_back({label, action, false});
}

void Menu::add_exit_item(const std::string& label) {
    items.push_back({label, nullptr, true});
}

void Menu::display() const {
    for (size_t i = 0; i < items.size(); ++i) {
        std::cout << COLOR_GREEN << (i + 1) << ". "
                  << COLOR_RESET << items[i].label << "\n";
    }
    std::cout << COLOR_YELLOW << "> " << COLOR_RESET;
}

int Menu::get_selection() const {
    return get_int("", 1, static_cast<int>(items.size()));
}

bool Menu::run(const Detection::DetectionResult& context) {
    // std::cout << "\033[2J\033[H";

    #ifndef _WIN32
    if (!title.empty()) {
        std::cout << COLOR_RED << print_title() << COLOR_RESET << "\n\n";
    }
    #endif

    display();
    int choice = get_selection();

    if (items[choice - 1].is_exit) {
        return false;
    }

    if (items[choice - 1].action) {
        items[choice - 1].action(context);
    }

    return true;
}
