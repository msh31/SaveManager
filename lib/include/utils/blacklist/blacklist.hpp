#pragma once

// for savemgr.ignore files
enum class IgnoreKind { Extension, Filename, Directory };
struct IgnoreRule {
        IgnoreKind kind;
        std::string value;
};

struct Blacklist {
    public:
        bool init( );
        void save( );

        bool is_blacklisted( const std::string& game_name ) const;

        const std::unordered_set<std::string> games( ) const;

        void add( const std::string& name );
        void remove( const std::string& name );

        // savemgr-ignore subsystem
        static std::vector<IgnoreRule> parse_ignore_file( const fs::path& f );
        static bool is_ignored( const fs::path& file, const std::vector<IgnoreRule>& rules );

    private:
        std::unordered_set<std::string> m_blacklisted_games;
        mutable std::mutex m_blacklist_mutex;

        static std::optional<IgnoreRule> parse_ignore_line( std::string_view l );
};
