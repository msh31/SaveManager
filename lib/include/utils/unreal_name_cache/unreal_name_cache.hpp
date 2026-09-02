#pragma once
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// remembers appid -> name pairs resolved from a local Steam manifest so the
// name survives after the game is uninstalled and its .acf disappears
struct UnrealNameCache {
    public:
        bool init( );
        std::optional<std::string> get( uint32_t appid ) const;
        void remember( uint32_t appid, const std::string& name );

        // for native (non-wine) saves, where all we have is the project folder name, not an appid
        std::optional<std::string> get_by_folder( const std::string& folder_name ) const;
        void remember_by_folder( const std::string& folder_name, const std::string& name );

    private:
        void save( ) const;

        std::unordered_map<uint32_t, std::string> m_cache = { };
        std::unordered_map<std::string, std::string> m_folder_cache = { };
        mutable std::mutex m_mutex;
};
