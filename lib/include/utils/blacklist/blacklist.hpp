#pragma once

struct Blacklist {
    public:
        bool init( );
        void save( );

        bool is_blacklisted( const std::string& game_name );

    private:
        std::unordered_set<std::string> m_blacklisted_games;
};
