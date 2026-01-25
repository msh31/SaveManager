#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <optional>
#include <fstream>

namespace fs = std::filesystem;

//linux only for now
std::optional<fs::path> getSteamLocation() {
    std::string home = std::getenv("HOME");
    std::string path1 = home + "/.steam/steam/steamapps/libraryfolders.vdf";
    std::string path2 = home + "/.local/share/Steam/steamapps/libraryfolders.vdf";

    if(fs::exists(path1)) {
        return path1;
    }

    if(fs::exists(path2)) {
        return path2;
    }

    return std::nullopt;
}

std::vector<fs::path> getLibraryFolders() {
    auto vdf_file = getSteamLocation();
    std::vector<fs::path> libraries;

    if(!vdf_file) {
        return {};
    }

    std::ifstream file = vdf_file.value();
    std::string line;

    if(!file.is_open()) {
        return {};
    }

    while (std::getline(file, line)) {
        if(line.find("\"path\"") != std::string::npos) {
            size_t first_quote = line.find('"');
            size_t second_quote = line.find('"', first_quote + 1);
            size_t third_quote = line.find('"', second_quote + 1);
            size_t fourth_quote = line.find('"', third_quote + 1);

            if(fourth_quote == std::string::npos) {
                continue;
            }

            std::string path_value = line.substr(third_quote + 1, fourth_quote - third_quote - 1);
            libraries.push_back(path_value);
        }
    }

    file.close();
    return libraries;
}

int main(int argc, char* argv[]) {
    auto libraries = getLibraryFolders();

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
