#include "input_validator.hpp"

void clear_input_error() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int get_int(const std::string& prompt) {
    int value = 0;

    if (!prompt.empty()) {
        std::cout << prompt;
    }

    std::cin >> value;

    while (std::cin.fail()) {
        clear_input_error();
        std::cerr << "Invalid input, try again.\n";
        if (!prompt.empty()) {
            std::cout << prompt;
        }
        std::cin >> value;
    }

    return value;
}

int get_int(const std::string& prompt, int min, int max) {
    int value = 0;

    std::cout << prompt;
    std::cin >> value;

    while (std::cin.fail() || value < min || value > max) {
        if (std::cin.fail()) {
            clear_input_error();
            std::cerr << "Invalid input, try again.\n";
        } else {
            std::cerr << "Invalid selection (must be between "
                      << min << " and " << max << ")\n";
        }
        std::cout << " > ";
        std::cin >> value;
    }

    return value;
}
