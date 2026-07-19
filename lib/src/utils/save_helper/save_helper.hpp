#pragma once
#include <types.hpp>

enum class SaveRoot { DOCUMENTS, LOCAL_APPDATA, LOCAL_APPDATA_LOW, PROGRAM_DATA, SAVED_GAMES };
struct SaveLocation {
        std::string game_name;
        SaveRoot root_path;
        fs::path relative_path;
        std::optional<std::array<char, 4>> header_bytes;
        bool show_parent_path = false;
};

namespace save {
    fs::path resolve_root( SaveRoot sr );

    // shared by any table-driven detector (see CElectronicArtsDetector for an example)
    std::vector<Game> scan_locations(
        const std::unordered_map<SaveRoot, fs::path>& roots, const std::vector<SaveLocation>& table,
        PlatformType type, std::string_view platform_label );
};
