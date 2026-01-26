#include <iostream>

#include "detection/detection.hpp"

// Detection detect;

int main(int argc, char* argv[]) {
    auto libraries = Detection::getLibraryFolders();
    if(libraries.empty()) {
        std::cout << "No steam libraries found\n";
        return 1;
    }

    std::cout << "Found libraries:\n";
    for(const auto& lib : libraries) {
        std::cout << lib << "\n";
    }

    // if (argc < 2) {
    //     std::cout << "Usage: savemanager\n";
    //     return 1;
    // }
    return 0;
}
