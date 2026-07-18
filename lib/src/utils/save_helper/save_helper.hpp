#pragma once

enum class SaveRoot { DOCUMENTS, LOCAL_APPDATA, LOCAL_APPDATA_LOW, PROGRAM_DATA, SAVED_GAMES };
struct SaveLocation {
        std::string game_name;
        SaveRoot root_path;
        fs::path relative_path;
        std::optional<std::array<char, 4>> header_bytes;
};

namespace save {
    fs::path resolve_root( SaveRoot sr );
};
