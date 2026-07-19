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

    private:
        void save( ) const;

        std::unordered_map<uint32_t, std::string> m_cache = { };
        mutable std::mutex m_mutex;
};
